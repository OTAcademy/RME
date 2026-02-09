#include "ui/replace_tool/rule_builder_panel.h"
#include "ui/theme.h"
#include "game/items.h"
#include "ui/gui.h"

#include "app/managers/version_manager.h"
#include "util/nvg_utils.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/msgdlg.h>
#include <nanovg.h>
#include <format>
#include <cmath>
#include "rendering/core/text_renderer.h"
#include "ui/replace_tool/rule_card_renderer.h"

// Layout Constants
static const int CARD_PADDING = 20;
static const int CARD_MARGIN_X = 10;
static const int CARD_MARGIN_Y = 10;
static const int HEADER_HEIGHT = 40;
static const int ITEM_SIZE = 56;
static const int ITEM_H = 110;
static const int ITEM_SPACING = 10;
static const int ARROW_WIDTH = 60;
static const int SECTION_GAP = 20;
static const int CARD_W = ITEM_SIZE + 20;
static const int GHOST_SLOT_WIDTH = CARD_W;

// ----------------------------------------------------------------------------
// ItemDropTarget
// ----------------------------------------------------------------------------

RuleBuilderPanel::ItemDropTarget::ItemDropTarget(RuleBuilderPanel* panel) : m_panel(panel) { }

bool RuleBuilderPanel::ItemDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data) {
	if (!data.StartsWith("RME_ITEM:")) {
		return false;
	}

	unsigned long idVal;
	if (!data.AfterFirst(':').ToULong(&idVal)) {
		return false;
	}
	uint16_t itemId = (uint16_t)idVal;

	RuleBuilderPanel::HitResult hit = m_panel->HitTest(x, y);

	if (hit.type == RuleBuilderPanel::HitResult::Source) {
		if (hit.ruleIndex >= 0 && hit.ruleIndex < (int)m_panel->m_rules.size()) {
			m_panel->m_rules[hit.ruleIndex].fromId = itemId;
			m_panel->m_listener->OnRuleChanged();
			m_panel->Refresh();
			return true;
		}
	} else if (hit.type == RuleBuilderPanel::HitResult::Target || hit.type == RuleBuilderPanel::HitResult::DeleteTarget) {
		if (hit.ruleIndex >= 0 && hit.ruleIndex < (int)m_panel->m_rules.size()) {
			// Replace existing target
			if (hit.targetIndex >= 0 && hit.targetIndex < m_panel->m_rules[hit.ruleIndex].targets.size()) {
				m_panel->m_rules[hit.ruleIndex].targets[hit.targetIndex].id = itemId;
				m_panel->m_listener->OnRuleChanged();
				m_panel->Refresh();
				return true;
			}
		}
	} else if (hit.type == RuleBuilderPanel::HitResult::AddTarget) { // Drag to [+] slot
		if (hit.ruleIndex >= 0 && hit.ruleIndex < (int)m_panel->m_rules.size()) {
			// TRASH LOGIC: Reject if has trash
			bool hasTrash = false;
			for (const auto& t : m_panel->m_rules[hit.ruleIndex].targets) {
				if (t.id == TRASH_ITEM_ID) {
					hasTrash = true;
				}
			}
			if (hasTrash) {
				return false;
			}

			m_panel->m_rules[hit.ruleIndex].targets.push_back({ itemId, 0 });
			m_panel->DistributeProbabilities(hit.ruleIndex);
			m_panel->m_listener->OnRuleChanged();
			m_panel->LayoutRules();
			m_panel->Refresh();
			return true;
		}
	} else if (hit.type == RuleBuilderPanel::HitResult::NewRule) {
		// Create a new rule with this item as Source
		ReplacementRule newRule;
		newRule.fromId = itemId;
		// Initialize with empty targets (User needs to add them) or maybe auto-add the same item?
		// User said "Drop New Source Here", so empty targets is fine.
		m_panel->m_rules.push_back(newRule);
		m_panel->m_listener->OnRuleChanged();
		m_panel->LayoutRules();
		m_panel->Refresh();
		return true;
	}

	return false;
}

wxDragResult RuleBuilderPanel::ItemDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
	RuleBuilderPanel::HitResult hit = m_panel->HitTest(x, y);
	m_panel->m_dragHover = hit;
	m_panel->Refresh();
	return def;
}

void RuleBuilderPanel::ItemDropTarget::OnLeave() {
	m_panel->m_dragHover = { RuleBuilderPanel::HitResult::None, -1, -1 };
	m_panel->Refresh();
}

// ----------------------------------------------------------------------------
// RuleBuilderPanel
// ----------------------------------------------------------------------------

RuleBuilderPanel::RuleBuilderPanel(wxWindow* parent, Listener* listener) :
	NanoVGCanvas(parent, wxID_ANY, wxVSCROLL | wxWANTS_CHARS),
	m_listener(listener) {

	SetDropTarget(new ItemDropTarget(this));

	Bind(wxEVT_SIZE, &RuleBuilderPanel::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_LEFT_DCLICK, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_MOTION, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_LEAVE_WINDOW, &RuleBuilderPanel::OnMouse, this);

	m_dragHover = { HitResult::None, -1, -1 };
}

RuleBuilderPanel::~RuleBuilderPanel() { }

void RuleBuilderPanel::Clear() {
	m_rules.clear();
	LayoutRules();
	Refresh();
}

void RuleBuilderPanel::SetRules(const std::vector<ReplacementRule>& rules) {
	m_rules = rules;
	LayoutRules();
	Refresh();
}

std::vector<ReplacementRule> RuleBuilderPanel::GetRules() const {
	return m_rules;
}

void RuleBuilderPanel::LayoutRules() {
	int width = GetClientSize().x;
	if (width <= 0) {
		width = FromDIP(500);
	}

	int totalHeight = HEADER_HEIGHT + CARD_MARGIN_Y;
	for (size_t i = 0; i < m_rules.size(); ++i) {
		totalHeight += GetRuleHeight(i, width) + CARD_MARGIN_Y;
	}
	totalHeight += FromDIP(100); // Space for New Rule area
	UpdateScrollbar(totalHeight);
}

int RuleBuilderPanel::GetRuleHeight(int index, int width) const {
	if (index < 0 || index >= (int)m_rules.size()) {
		return FromDIP(ITEM_H);
	}

	const float TARGET_START_X = CARD_PADDING + CARD_W + 10 + ARROW_WIDTH;
	float availableWidth = width - CARD_MARGIN_X * 2 - TARGET_START_X - CARD_PADDING;
	if (availableWidth < CARD_W) {
		availableWidth = CARD_W;
	}

	int columns = std::max(1, (int)(availableWidth / (CARD_W + ITEM_SPACING)));
	int targetCount = m_rules[index].targets.size();

	bool hasTrash = false;
	for (const auto& t : m_rules[index].targets) {
		if (t.id == TRASH_ITEM_ID) {
			hasTrash = true;
		}
	}
	if (!hasTrash) {
		targetCount++; // [+] slot
	}

	int rows = std::max(1, (targetCount + columns - 1) / columns);
	int itemH = FromDIP(ITEM_H);
	return rows * (itemH + ITEM_SPACING) + CARD_PADDING * 2;
}

int RuleBuilderPanel::GetRuleY(int index, int width) const {
	int y = HEADER_HEIGHT + CARD_MARGIN_Y;
	for (int i = 0; i < index; ++i) {
		y += GetRuleHeight(i, width) + CARD_MARGIN_Y;
	}
	return y;
}

void RuleBuilderPanel::DistributeProbabilities(int ruleIndex) {
	if (ruleIndex < 0 || ruleIndex >= m_rules.size()) {
		return;
	}
	auto& targets = m_rules[ruleIndex].targets;

	if (targets.empty()) {
		return;
	}
	int count = targets.size();
	// Use floating point accumulation for fairer distribution of remainders
	double step = 100.0 / count;
	double accumulated = 0.0;

	for (size_t i = 0; i < targets.size(); ++i) {
		accumulated += step;
		int currentTotal = (int)std::round(accumulated);
		int prevTotal = (int)std::round(accumulated - step);
		targets[i].probability = currentTotal - prevTotal;
	}
}

wxSize RuleBuilderPanel::DoGetBestClientSize() const {
	return wxSize(FromDIP(500), FromDIP(400));
}

void RuleBuilderPanel::OnSize(wxSizeEvent& event) {
	// Guard against redundant size events to prevent potential loop/jitter
	if (GetClientSize() != m_lastSize) {
		m_lastSize = GetClientSize();
		LayoutRules();
		Refresh();
	}
	event.Skip();
}

void RuleBuilderPanel::OnMouse(wxMouseEvent& event) {
	if (event.LeftDown()) {
		HitResult hit = HitTest(event.GetX(), event.GetY());
		if (hit.type == HitResult::DeleteRule && hit.ruleIndex != -1) {
			m_rules.erase(m_rules.begin() + hit.ruleIndex);
			if (m_listener) {
				m_listener->OnRuleChanged();
			}
			LayoutRules();
			Refresh();
		} else if (hit.type == HitResult::DeleteTarget && hit.ruleIndex != -1 && hit.targetIndex != -1) {
			m_rules[hit.ruleIndex].targets.erase(m_rules[hit.ruleIndex].targets.begin() + hit.targetIndex);
			DistributeProbabilities(hit.ruleIndex);
			if (m_listener) {
				m_listener->OnRuleChanged();
			}
			LayoutRules();
			Refresh();
		} else if (hit.type == HitResult::AddTarget && hit.ruleIndex != -1) {
			if (m_rules[hit.ruleIndex].targets.empty()) {
				ReplacementTarget t;
				t.id = TRASH_ITEM_ID;
				t.probability = 100;
				m_rules[hit.ruleIndex].targets.push_back(t);
				if (m_listener) {
					m_listener->OnRuleChanged();
				}
				Refresh();
			}
		} else if (hit.type == HitResult::ClearRules) {
			wxMessageDialog dlg(this, "Are you sure you want to clear all rules? This action cannot be undone.", "Clear Rules", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
			if (dlg.ShowModal() == wxID_YES) {
				Clear();
				if (m_listener) {
					m_listener->OnClearRules();
				}
			}
		}
	}

	if (event.Moving()) {
		HitResult hit = HitTest(event.GetX(), event.GetY());
		if (hit.type != m_dragHover.type || hit.ruleIndex != m_dragHover.ruleIndex || hit.targetIndex != m_dragHover.targetIndex) {
			m_dragHover = hit;
			Refresh();
		}
	}
	event.Skip();
}

RuleBuilderPanel::HitResult RuleBuilderPanel::HitTest(int x, int y) const {
	int scrollPos = GetScrollPosition();
	int absY = y + scrollPos;
	int width = GetClientSize().x;

	const float ITEM_HEIGHT = FromDIP(ITEM_H);

	// Header blocked area
	if (y < HEADER_HEIGHT) {
		// Only allow clicking the Clear button in the header
		const int CLEAR_BTN_W = FromDIP(100);
		if (x > width - CLEAR_BTN_W - 10) {
			return { HitResult::ClearRules, -1, -1 };
		}
		// Otherwise, we are clicking the header background/labels - consume the hit so we don't click rules underneath
		return { HitResult::None, -1, -1 };
	}

	// Check Rules
	for (size_t i = 0; i < m_rules.size(); ++i) {
		int ruleH = GetRuleHeight(i, width);
		int ruleY = GetRuleY(i, width);
		wxRect card(CARD_MARGIN_X, ruleY, width - CARD_MARGIN_X * 2, ruleH);

		if (card.Contains(x, absY)) {
			int localX = x - card.x;
			int localY = absY - card.y;

			// Delete Rule Button (Top Right)
			if (localX > card.width - 24 && localY < 24) {
				return { HitResult::DeleteRule, (int)i, -1 };
			}

			// Source Item (Left, always in first row logically)
			float startX = CARD_PADDING;
			float sourceY = CARD_PADDING; // Vertical top in card

			if (localX >= startX && localX <= startX + CARD_W && localY >= sourceY && localY <= sourceY + ITEM_HEIGHT) {
				return { HitResult::Source, (int)i, -1 };
			}

			// Targets (Wrapping)
			float targetStartX = startX + CARD_W + 10 + ARROW_WIDTH;
			float availableWidth = card.width - targetStartX - CARD_PADDING;
			int columns = std::max(1, (int)(availableWidth / (CARD_W + ITEM_SPACING)));

			for (size_t t = 0; t < m_rules[i].targets.size(); ++t) {
				int row = t / columns;
				int col = t % columns;
				float tx = targetStartX + col * (CARD_W + ITEM_SPACING);
				float ty = CARD_PADDING + row * (ITEM_HEIGHT + ITEM_SPACING);

				if (localX >= tx && localX <= tx + CARD_W && localY >= ty && localY <= ty + ITEM_HEIGHT) {
					return { HitResult::DeleteTarget, (int)i, (int)t };
				}
			}

			// Add Target Slot
			bool hasTrash = false;
			for (const auto& t : m_rules[i].targets) {
				if (t.id == TRASH_ITEM_ID) {
					hasTrash = true;
				}
			}

			if (!hasTrash) {
				int tIdx = m_rules[i].targets.size();
				int row = tIdx / columns;
				int col = tIdx % columns;
				float tx = targetStartX + col * (CARD_W + ITEM_SPACING);
				float ty = CARD_PADDING + row * (ITEM_HEIGHT + ITEM_SPACING);

				if (localX >= tx && localX <= tx + GHOST_SLOT_WIDTH && localY >= ty && localY <= ty + ITEM_HEIGHT) {
					return { HitResult::AddTarget, (int)i, -1 };
				}
			}

			return { HitResult::None, (int)i, -1 };
		}
	}

	// Clear Rules Button (Far Right of Header)
	const int CLEAR_BTN_W = FromDIP(100);
	if (y < HEADER_HEIGHT && x > width - CLEAR_BTN_W - 10) {
		return { HitResult::ClearRules, -1, -1 };
	}

	// New Rule Area (At the bottom)
	int newRuleY = GetRuleY(m_rules.size(), width) + CARD_MARGIN_Y;
	float dropH = 60.0f; // Must match OnNanoVGPaint
	if (absY >= newRuleY && absY <= newRuleY + dropH) {
		return { HitResult::NewRule, -1, -1 };
	}

	return { HitResult::None, -1, -1 };
}

// Methods removed, using RuleCardRenderer instead

void RuleBuilderPanel::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	int scrollPos = GetScrollPosition();

	// Note: NanoVGCanvas already applies a convert translation of nvgTranslate(0, -scrollPos).
	// To draw fixed elements (Background, Header), we must undo this translation.

	// 1. Draw Fixed Elements (Background + Header)
	nvgSave(vg);
	nvgTranslate(vg, 0, (float)scrollPos); // Undo base scroll

	// Full screen BG (Fixed)
	nvgBeginPath(vg);
	nvgRect(vg, 0, 0, width, height);
	nvgFillColor(vg, nvgRGBA(30, 30, 30, 255)); // Darker background
	nvgFill(vg);

	DrawHeader(vg, width);
	DrawClearButton(vg, width);
	nvgRestore(vg);

	// 2. Draw Content (Implicitly scrolled by base class)
	nvgSave(vg);
	// No extra translate needed!

	for (size_t i = 0; i < m_rules.size(); ++i) {
		int ruleY = GetRuleY(i, width);
		DrawRuleCard(vg, i, ruleY, width);
	}

	int newRuleY = GetRuleY(m_rules.size(), width) + CARD_MARGIN_Y;
	DrawNewRuleArea(vg, width, newRuleY);

	nvgRestore(vg);
}

void RuleBuilderPanel::DrawHeader(NVGcontext* vg, float width) {
	NVGcolor subTextCol = nvgRGBA(150, 150, 150, 255);

	nvgFontSize(vg, 10.0f);
	nvgFillColor(vg, subTextCol);
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

	float headerY = HEADER_HEIGHT / 2.0f;
	float sourceLabelX = CARD_MARGIN_X + CARD_PADDING;
	nvgText(vg, sourceLabelX, headerY, "IF FOUND", nullptr);

	float targetLabelX = sourceLabelX + CARD_W + 10 + ARROW_WIDTH;
	nvgText(vg, targetLabelX, headerY, "REPLACE WITH", nullptr);

	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg); // Is this needed?
}

void RuleBuilderPanel::DrawClearButton(NVGcontext* vg, float width) {
	const int CLEAR_BTN_W = FromDIP(100);
	const int CLEAR_BTN_H = FromDIP(24);
	float cbX = width - CLEAR_BTN_W - 10;
	float cbY = (HEADER_HEIGHT - CLEAR_BTN_H) / 2.0f;

	bool hoverClear = (m_dragHover.type == HitResult::ClearRules);

	nvgBeginPath(vg);
	nvgRoundedRect(vg, cbX, cbY, CLEAR_BTN_W, CLEAR_BTN_H, 4);
	if (hoverClear) {
		nvgFillColor(vg, nvgRGBA(180, 40, 40, 255));
	} else {
		nvgFillColor(vg, nvgRGBA(60, 60, 60, 255));
	}
	nvgFill(vg);

	nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
	nvgFontSize(vg, 11.0f);
	nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgText(vg, cbX + CLEAR_BTN_W / 2.0f, cbY + CLEAR_BTN_H / 2.0f, "CLEAR RULES", nullptr);
}

void RuleBuilderPanel::DrawRuleCard(NVGcontext* vg, int ruleIndex, int y, int width) {
	int ruleH = GetRuleHeight(ruleIndex, width);
	// Card BG
	nvgBeginPath(vg);
	nvgRoundedRect(vg, CARD_MARGIN_X, y, width - CARD_MARGIN_X * 2, ruleH, 4);
	nvgFillColor(vg, nvgRGBA(50, 50, 50, 255));
	nvgFill(vg);
	nvgStrokeColor(vg, nvgRGBA(70, 70, 70, 255));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	float startX = CARD_MARGIN_X + CARD_PADDING;
	float itemH = FromDIP(ITEM_H);
	float itemY = y + CARD_PADDING;

	DrawRuleSource(vg, ruleIndex, startX, itemY, itemH);

	float arrowX = startX + (ITEM_SIZE + 20) + 10;
	DrawRuleArrow(vg, arrowX, itemY, itemH);

	float targetStartX = arrowX + ARROW_WIDTH;
	float availableWidthForTargets = (width - CARD_MARGIN_X * 2) - (targetStartX - CARD_MARGIN_X) - CARD_PADDING;
	DrawRuleTargets(vg, ruleIndex, targetStartX, y, availableWidthForTargets, itemH);

	// Delete Rule Button
	NVGcolor subTextCol = nvgRGBA(150, 150, 150, 255);
	nvgFillColor(vg, subTextCol);
	nvgFontSize(vg, 16.0f);
	nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	if (m_dragHover.type == HitResult::DeleteRule && m_dragHover.ruleIndex == ruleIndex) {
		nvgFillColor(vg, nvgRGBA(255, 80, 80, 255));
	}
	nvgText(vg, CARD_MARGIN_X + (width - CARD_MARGIN_X * 2) - 15, y + 15, "X", nullptr);
}

void RuleBuilderPanel::DrawRuleSource(NVGcontext* vg, int ruleIndex, float x, float y, float h) {
	bool hoverSource = (m_dragHover.type == HitResult::Source && m_dragHover.ruleIndex == ruleIndex);
	const auto& rule = m_rules[ruleIndex];
	RuleCardRenderer::DrawRuleItemCard(this, vg, x, y, ITEM_SIZE + 20, h, rule.fromId, hoverSource, false, false);
}

void RuleBuilderPanel::DrawRuleArrow(NVGcontext* vg, float x, float y, float h) {
	float arrowYCenter = y + h / 2.0f;
	NVGcolor subTextCol = nvgRGBA(150, 150, 150, 255);

	nvgBeginPath(vg);
	nvgMoveTo(vg, x, arrowYCenter);
	nvgLineTo(vg, x + ARROW_WIDTH - 20, arrowYCenter);
	nvgStrokeColor(vg, subTextCol);
	nvgStrokeWidth(vg, 2.0f);
	nvgStroke(vg);

	float tailX = x + ARROW_WIDTH - 20;
	nvgBeginPath(vg);
	nvgMoveTo(vg, tailX - 5, arrowYCenter - 5);
	nvgLineTo(vg, tailX, arrowYCenter);
	nvgLineTo(vg, tailX - 5, arrowYCenter + 5);
	nvgStroke(vg);
}

void RuleBuilderPanel::DrawRuleTargets(NVGcontext* vg, int ruleIndex, float startX, float ruleY, float width, float h) {
	const auto& rule = m_rules[ruleIndex];
	int columns = std::max(1, (int)(width / (ITEM_SIZE + 20 + ITEM_SPACING)));
	NVGcolor accentCol = nvgRGBA(Theme::Get(Theme::Role::Accent).Red(), Theme::Get(Theme::Role::Accent).Green(), Theme::Get(Theme::Role::Accent).Blue(), 255);

	bool hasTrash = false;
	for (size_t j = 0; j < rule.targets.size(); ++j) {
		const auto& target = rule.targets[j];
		int row = j / columns;
		int col = j % columns;
		float tx = startX + col * (ITEM_SIZE + 20 + ITEM_SPACING);
		float ty = ruleY + CARD_PADDING + row * (h + ITEM_SPACING);

		bool isThisHovered = (m_dragHover.ruleIndex == ruleIndex && m_dragHover.targetIndex == (int)j && (m_dragHover.type == HitResult::Target || m_dragHover.type == HitResult::DeleteTarget));
		bool isTrash = (target.id == TRASH_ITEM_ID);
		if (isTrash) {
			hasTrash = true;
		}

		RuleCardRenderer::DrawRuleItemCard(this, vg, tx, ty, ITEM_SIZE + 20, h, target.id, isThisHovered, isTrash, isThisHovered, target.probability);
	}

	// Ghost Slot
	if (!hasTrash) {
		int tIdx = rule.targets.size();
		int row = tIdx / columns;
		int col = tIdx % columns;
		float tx = startX + col * (ITEM_SIZE + 20 + ITEM_SPACING);
		float ty = ruleY + CARD_PADDING + row * (h + ITEM_SPACING);

		bool hoverNewTarget = (m_dragHover.type == HitResult::AddTarget && m_dragHover.ruleIndex == ruleIndex);

		nvgBeginPath(vg);
		nvgRoundedRect(vg, tx, ty, ITEM_SIZE + 20, h, 4);
		if (hoverNewTarget) {
			nvgStrokeColor(vg, accentCol);
			nvgStrokeWidth(vg, 2.0f);
		} else {
			nvgStrokeColor(vg, nvgRGBA(80, 80, 80, 255));
			nvgStrokeWidth(vg, 1.0f);
		}
		nvgStroke(vg);

		if (hoverNewTarget && rule.targets.empty()) {
			RuleCardRenderer::DrawTrashIcon(vg, tx + (ITEM_SIZE + 20 - 32.0f) / 2, ty + 8, 32.0f, true);
			nvgFillColor(vg, nvgRGBA(255, 100, 100, 255));
			nvgFontSize(vg, 11.0f);
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
			nvgText(vg, tx + (ITEM_SIZE + 20) / 2, ty + 44, "REMOVE", nullptr);
		} else {
			nvgFontSize(vg, 30.0f);
			nvgFillColor(vg, hoverNewTarget ? accentCol : nvgRGBA(100, 100, 100, 255));
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
			nvgText(vg, tx + (ITEM_SIZE + 20) / 2, ty + h / 2, "+", nullptr);
		}
	}
}

void RuleBuilderPanel::DrawNewRuleArea(NVGcontext* vg, float width, float y) {
	NVGcolor cardBgTop = nvgRGBA(45, 45, 50, 255);
	NVGcolor cardBgBot = nvgRGBA(40, 40, 45, 255);
	NVGcolor borderColor = nvgRGBA(100, 100, 100, 255);
	NVGcolor accentCol = nvgRGBA(Theme::Get(Theme::Role::Accent).Red(), Theme::Get(Theme::Role::Accent).Green(), Theme::Get(Theme::Role::Accent).Blue(), 255);
	NVGcolor subTextCol = nvgRGBA(150, 150, 150, 255);

	float dropH = 60.0f;
	float cardX = CARD_MARGIN_X;
	float cardW = width - CARD_MARGIN_X * 2;

	NVGpaint bgPaint = nvgLinearGradient(vg, cardX, y, cardX, y + dropH, cardBgTop, cardBgBot);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, cardX, y, cardW, dropH, 4);
	nvgFillPaint(vg, bgPaint);
	nvgFill(vg);

	if (m_dragHover.type == HitResult::NewRule) {
		nvgStrokeColor(vg, accentCol);
		nvgStrokeWidth(vg, 2.0f);
		nvgFillColor(vg, nvgRGBAf(accentCol.r, accentCol.g, accentCol.b, 0.1f));
		nvgFill(vg);
	} else {
		nvgStrokeColor(vg, borderColor);
		nvgStrokeWidth(vg, 1.0f);
	}
	nvgStroke(vg);

	nvgFontSize(vg, 14.0f);
	nvgFillColor(vg, subTextCol);
	nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgText(vg, width / 2.0f, y + dropH / 2.0f, "Drop Item Here to Add New Rule", nullptr);
}
