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
		} else if (hit.type == HitResult::SaveRule) {
			if (m_listener) {
				m_listener->OnSaveRule();
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
		const int BTN_W = FromDIP(80);
		const int GAP = FromDIP(10);

		// Clear Button (Far right)
		if (x > width - BTN_W - GAP) {
			return { HitResult::ClearRules, -1, -1 };
		}
		// Save Button (Left of Clear)
		if (x > width - (BTN_W * 2) - (GAP * 2) && x < width - BTN_W - GAP) {
			return { HitResult::SaveRule, -1, -1 };
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
				float tx = targetStartX + col * (RuleCardRenderer::CARD_W + RuleCardRenderer::ITEM_SPACING);
				float ty = RuleCardRenderer::CARD_PADDING + row * (ITEM_HEIGHT + RuleCardRenderer::ITEM_SPACING);

				if (localX >= tx && localX <= tx + RuleCardRenderer::CARD_W && localY >= ty && localY <= ty + ITEM_HEIGHT) {
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
				float tx = targetStartX + col * (RuleCardRenderer::CARD_W + RuleCardRenderer::ITEM_SPACING);
				float ty = RuleCardRenderer::CARD_PADDING + row * (ITEM_HEIGHT + RuleCardRenderer::ITEM_SPACING);

				if (localX >= tx && localX <= tx + RuleCardRenderer::CARD_W && localY >= ty && localY <= ty + ITEM_HEIGHT) {
					return { HitResult::AddTarget, (int)i, -1 };
				}
			}

			return { HitResult::None, (int)i, -1 };
		}
	}

	// Clear Rules Button (Far Right of Header)
	// Handled in block above

	// New Rule Area (At the bottom)
	int newRuleY = GetRuleY(m_rules.size(), width) + RuleCardRenderer::CARD_MARGIN_Y;
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

	RuleCardRenderer::DrawHeader(vg, width);
	RuleCardRenderer::DrawClearButton(vg, width, m_dragHover.type == HitResult::ClearRules);
	RuleCardRenderer::DrawSaveButton(vg, width, m_dragHover.type == HitResult::SaveRule);
	nvgRestore(vg);

	// 2. Draw Content (Implicitly scrolled by base class)
	for (size_t i = 0; i < m_rules.size(); ++i) {
		int ruleY = GetRuleY(i, width);
		bool hoverDel = (m_dragHover.type == HitResult::DeleteRule && m_dragHover.ruleIndex == (int)i);
		int dragType = (m_dragHover.ruleIndex == (int)i) ? (int)m_dragHover.type : 0;
		int dragIdx = (m_dragHover.ruleIndex == (int)i) ? m_dragHover.targetIndex : -1;

		RuleCardRenderer::DrawRuleCard(this, vg, (int)i, ruleY, width, hoverDel, dragIdx, dragType);
	}

	int newRuleY = GetRuleY(m_rules.size(), width) + RuleCardRenderer::CARD_MARGIN_Y;
	RuleCardRenderer::DrawNewRuleArea(vg, width, newRuleY, m_dragHover.type == HitResult::NewRule);
}

// ----------------------------------------------------------------------------
// Removed all private Draw methods (now in RuleCardRenderer)
// ----------------------------------------------------------------------------
