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

static const uint16_t TRASH_ITEM_ID = 0xFFFF;

// Layout Constants
static const int CARD_PADDING = 20;
static const int CARD_MARGIN_X = 10;
static const int CARD_MARGIN_Y = 10;
static const int HEADER_HEIGHT = 40;
static const int ITEM_SIZE = 48;
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
	NanoVGCanvas(parent, wxID_ANY),
	m_listener(listener),
	m_pulseTimer(this) {

	SetDropTarget(new ItemDropTarget(this));

	Bind(wxEVT_SIZE, &RuleBuilderPanel::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_MOTION, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_LEAVE_WINDOW, &RuleBuilderPanel::OnMouse, this);
	m_pulseTimer.Bind(wxEVT_TIMER, &RuleBuilderPanel::OnPulseTimer, this);

	m_dragHover = { HitResult::None, -1, -1 };

	// Start pulse if empty
	if (m_rules.empty()) {
		m_pulseTimer.Start(30); // 30ms ~ 30fps
	}
}

RuleBuilderPanel::~RuleBuilderPanel() { }

void RuleBuilderPanel::OnPulseTimer(wxTimerEvent&) {
	if (IsShownOnScreen()) {
		Refresh();
	}
}

void RuleBuilderPanel::SetRules(const std::vector<ReplacementRule>& rules) {
	m_rules = rules;
	if (m_rules.empty() && !m_pulseTimer.IsRunning()) {
		m_pulseTimer.Start(30);
	} else if (!m_rules.empty() && m_pulseTimer.IsRunning()) {
		m_pulseTimer.Stop();
	}
	LayoutRules();
	Refresh();
}

std::vector<ReplacementRule> RuleBuilderPanel::GetRules() const {
	return m_rules;
}

void RuleBuilderPanel::Clear() {
	m_rules.clear();
	if (!m_pulseTimer.IsRunning()) {
		m_pulseTimer.Start(30);
	}
	LayoutRules();
	Refresh();
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
		return FromDIP(110);
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
	int itemH = FromDIP(88);
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
	int prob = 100 / count;
	int remainder = 100 % count;

	for (size_t i = 0; i < targets.size(); ++i) {
		targets[i].probability = prob;
		if (i < remainder) {
			targets[i].probability++;
		}
	}
}

wxSize RuleBuilderPanel::DoGetBestClientSize() const {
	return wxSize(FromDIP(500), FromDIP(400));
}

void RuleBuilderPanel::OnSize(wxSizeEvent& event) {
	LayoutRules();
	Refresh();
	event.Skip();
}

RuleBuilderPanel::HitResult RuleBuilderPanel::HitTest(int x, int y) const {
	int scrollPos = GetScrollPosition();
	int absY = y + scrollPos;
	int width = GetClientSize().x;

	const float ITEM_H = FromDIP(88);

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

			if (localX >= startX && localX <= startX + CARD_W && localY >= sourceY && localY <= sourceY + ITEM_H) {
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
				float ty = CARD_PADDING + row * (ITEM_H + ITEM_SPACING);

				if (localX >= tx && localX <= tx + CARD_W && localY >= ty && localY <= ty + ITEM_H) {
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
				float ty = CARD_PADDING + row * (ITEM_H + ITEM_SPACING);

				if (localX >= tx && localX <= tx + GHOST_SLOT_WIDTH && localY >= ty && localY <= ty + ITEM_H) {
					return { HitResult::AddTarget, (int)i, -1 };
				}
			}

			return { HitResult::None, (int)i, -1 };
		}
	}

	// New Rule Area
	int newRuleY = GetRuleY(m_rules.size(), width) + CARD_MARGIN_Y;
	if (absY >= newRuleY && absY <= newRuleY + FromDIP(60)) {
		return { HitResult::NewRule, -1, -1 };
	}

	return { HitResult::None, -1, -1 };
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
			Refresh();
		} else if (hit.type == HitResult::AddTarget && hit.ruleIndex != -1) {
			// Logic: If empty, click adds "Remove from Map" (Trash)
			// User logic: "when mouse is hovered... show 'remove from map' so user can click it"
			if (m_rules[hit.ruleIndex].targets.empty()) {
				ReplacementTarget t;
				t.id = 0xFFFF; // TRASH_ITEM_ID
				t.probability = 100;
				m_rules[hit.ruleIndex].targets.push_back(t);
				if (m_listener) {
					m_listener->OnRuleChanged();
				}
				Refresh();
			}
		}
	}

	// Hover tracking
	if (event.Moving()) {
		HitResult hit = HitTest(event.GetX(), event.GetY());
		// Only refresh if hover state changed significantly
		static HitResult lastHover = { HitResult::None, -1, -1 };
		if (hit.type != lastHover.type || hit.ruleIndex != lastHover.ruleIndex || hit.targetIndex != lastHover.targetIndex) {
			lastHover = hit;
			m_dragHover = hit; // Reuse drag hover for visual feedback
			Refresh();
		}
	}
	event.Skip();
}

void RuleBuilderPanel::DrawTrashIcon(NVGcontext* vg, float x, float y, float size, bool highlight) {
	nvgBeginPath(vg);
	// Simple Bin
	float w = size * 0.6f;
	float h = size * 0.7f;
	float ox = x + (size - w) / 2;
	float oy = y + (size - h) / 2;

	nvgFillColor(vg, highlight ? nvgRGBA(255, 80, 80, 255) : nvgRGBA(200, 50, 50, 255));

	// Body
	nvgRect(vg, ox, oy + 5, w, h - 5);
	// Lid
	nvgRect(vg, ox - 2, oy, w + 4, 3);
	// Handle
	nvgRect(vg, ox + w / 2 - 3, oy - 2, 6, 2);
	nvgFill(vg);
}

void RuleBuilderPanel::DrawRuleItemCard(NVGcontext* vg, float x, float y, float size, uint16_t id, bool highlight, bool isTrash, bool showDeleteOverlay, int probability) {
	float w = size + 20;
	float h = FromDIP(88); // Taller for text (ID + Chance)

	// Card BG Gradient (Match VirtualItemGrid: 60,60,65 -> 50,50,55)
	NVGpaint bgPaint = nvgLinearGradient(vg, x, y, x, y + h, nvgRGBA(60, 60, 65, 255), nvgRGBA(50, 50, 55, 255));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x, y, w, h, 4.0f);
	nvgFillPaint(vg, bgPaint);
	nvgFill(vg);

	// Border
	if (highlight) {
		nvgStrokeColor(vg, nvgRGBA(0, 120, 215, 255));
		nvgStrokeWidth(vg, 2.0f);
	} else {
		nvgStrokeColor(vg, nvgRGBA(60, 60, 70, 255));
		nvgStrokeWidth(vg, 1.0f);
	}
	nvgStroke(vg);

	float drawSize = 32.0f;

	// Content
	if (isTrash) {
		DrawTrashIcon(vg, x + (w - drawSize) / 2, y + 8, drawSize, highlight);
		nvgFillColor(vg, nvgRGBA(200, 200, 200, 255));
		nvgFontSize(vg, 12.0f);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		nvgText(vg, x + w / 2, y + 44, "REMOVE", nullptr);
	} else if (id != 0) {
		// Draw Item
		int tex = NvgUtils::CreateItemTexture(vg, id);
		if (tex > 0) {
			int tw, th;
			nvgImageSize(vg, tex, &tw, &th);
			float scale = drawSize / std::max(tw, th);
			float dw = tw * scale;
			float dh = th * scale;
			float dx = x + (w - dw) / 2;
			float dy = y + 8 + (drawSize - dh) / 2;

			NVGpaint imgPaint = nvgImagePattern(vg, dx, dy, dw, dh, 0.0f, tex, 1.0f);
			nvgBeginPath(vg);
			nvgRect(vg, dx, dy, dw, dh);
			nvgFillPaint(vg, imgPaint);
			nvgFill(vg);
		}

		// Text: "ID - Name"
		ItemType& it = g_items[id];
		wxString name = it.name;
		if (name.IsEmpty()) {
			name = "Item";
		}
		std::string label = std::format("{} - {}", id, name.ToStdString());

		nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
		nvgFontSize(vg, 12.0f);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		nvgText(vg, x + w / 2, y + 44, label.c_str(), nullptr);

		// Probability Line
		if (probability >= 0) {
			std::string probLabel = std::format("Chance: {}%", probability);
			nvgFillColor(vg, nvgRGBA(160, 160, 160, 255));
			nvgFontSize(vg, 11.0f);
			nvgText(vg, x + w / 2, y + 60, probLabel.c_str(), nullptr);
		}
	}

	// Delete Icon (Small Corner X)
	if (showDeleteOverlay) {
		float cx = x + w - 8;
		float cy = y + 8;
		float r = 8.0f;

		nvgBeginPath(vg);
		nvgCircle(vg, cx, cy, r);
		nvgFillColor(vg, nvgRGBA(200, 50, 50, 255));
		nvgFill(vg);

		nvgBeginPath(vg);
		float crossArr = r * 0.5f;
		nvgMoveTo(vg, cx - crossArr, cy - crossArr);
		nvgLineTo(vg, cx + crossArr, cy + crossArr);
		nvgMoveTo(vg, cx + crossArr, cy - crossArr);
		nvgLineTo(vg, cx - crossArr, cy + crossArr);
		nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
		nvgStrokeWidth(vg, 1.5f);
		nvgStroke(vg);
	}
}

void RuleBuilderPanel::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	int rowHeight = FromDIP(110);
	int scrollPos = GetScrollPosition();

	wxColour bgCol = Theme::Get(Theme::Role::Surface);
	NVGcolor nvgBg = nvgRGBA(bgCol.Red(), bgCol.Green(), bgCol.Blue(), 255);

	// Pulse alpha calculation
	float pulse = 0.0f;
	if (m_rules.empty()) {
		long long ms = wxGetLocalTimeMillis().GetValue();
		pulse = (sinf(ms * 0.005f) + 1.0f) * 0.5f; // 0 to 1
	}

	// Clear BG
	nvgBeginPath(vg);
	nvgRect(vg, 0, 0, width, height);
	nvgFillColor(vg, nvgRGBA(30, 30, 30, 255)); // Darker background
	nvgFill(vg);

	nvgSave(vg);
	nvgTranslate(vg, 0, -scrollPos);

	NVGcolor textCol = nvgRGBA(255, 255, 255, 255);
	NVGcolor subTextCol = nvgRGBA(150, 150, 150, 255);
	NVGcolor accentCol = nvgRGBA(Theme::Get(Theme::Role::Accent).Red(), Theme::Get(Theme::Role::Accent).Green(), Theme::Get(Theme::Role::Accent).Blue(), 255);

	const float ITEM_H = FromDIP(88); // Height of item card

	// Draw Fixed Headers
	nvgFontSize(vg, 10.0f);
	nvgFillColor(vg, subTextCol);
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);

	float headerY = HEADER_HEIGHT / 2.0f;
	float sourceLabelX = CARD_MARGIN_X + CARD_PADDING;
	nvgText(vg, sourceLabelX, headerY, "IF FOUND", nullptr);

	float targetLabelX = sourceLabelX + CARD_W + 10 + ARROW_WIDTH;
	nvgText(vg, targetLabelX, headerY, "REPLACE WITH", nullptr);

	// Separator line for static header
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0, HEADER_HEIGHT);
	nvgLineTo(vg, width, HEADER_HEIGHT);
	nvgStrokeColor(vg, nvgRGBA(60, 60, 60, 255));
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	nvgSave(vg);
	nvgTranslate(vg, 0, -scrollPos);

	for (size_t i = 0; i < m_rules.size(); ++i) {
		const auto& rule = m_rules[i];
		int ruleH = GetRuleHeight(i, width);
		int ruleY = GetRuleY(i, width);

		// Card BG
		nvgBeginPath(vg);
		nvgRoundedRect(vg, CARD_MARGIN_X, ruleY, width - CARD_MARGIN_X * 2, ruleH, 4);
		nvgFillColor(vg, nvgRGBA(50, 50, 50, 255));
		nvgFill(vg);
		nvgStrokeColor(vg, nvgRGBA(70, 70, 70, 255));
		nvgStrokeWidth(vg, 1.0f);
		nvgStroke(vg);

		// 1. Source Item (Centered vertically in first row of card)
		float startX = CARD_MARGIN_X + CARD_PADDING;
		float itemH = FromDIP(88);
		float itemY = ruleY + CARD_PADDING;

		bool hoverSource = (m_dragHover.type == HitResult::Source && m_dragHover.ruleIndex == i);
		DrawRuleItemCard(vg, startX, itemY, ITEM_SIZE, rule.fromId, hoverSource, false, false);

		// 2. Arrow
		float arrowX = startX + (ITEM_SIZE + 20) + 10;
		float arrowYCenter = itemY + itemH / 2.0f;

		nvgBeginPath(vg);
		nvgMoveTo(vg, arrowX, arrowYCenter);
		nvgLineTo(vg, arrowX + ARROW_WIDTH - 20, arrowYCenter);
		nvgStrokeColor(vg, subTextCol);
		nvgStrokeWidth(vg, 2.0f);
		nvgStroke(vg);

		float tailX = arrowX + ARROW_WIDTH - 20;
		nvgBeginPath(vg);
		nvgMoveTo(vg, tailX - 5, arrowYCenter - 5);
		nvgLineTo(vg, tailX, arrowYCenter);
		nvgLineTo(vg, tailX - 5, arrowYCenter + 5);
		nvgStroke(vg);

		// 3. Targets (Wrapping)
		float targetStartX = arrowX + ARROW_WIDTH;
		float availableWidthForTargets = (width - CARD_MARGIN_X * 2) - (targetStartX - CARD_MARGIN_X) - CARD_PADDING;
		int columns = std::max(1, (int)(availableWidthForTargets / (ITEM_SIZE + 20 + ITEM_SPACING)));

		bool hasTrash = false;
		for (size_t j = 0; j < rule.targets.size(); ++j) {
			const auto& target = rule.targets[j];
			int row = j / columns;
			int col = j % columns;
			float tx = targetStartX + col * (ITEM_SIZE + 20 + ITEM_SPACING);
			float ty = ruleY + CARD_PADDING + row * (itemH + ITEM_SPACING);

			bool isThisHovered = (m_dragHover.ruleIndex == i && m_dragHover.targetIndex == (int)j && (m_dragHover.type == HitResult::Target || m_dragHover.type == HitResult::DeleteTarget));
			bool isTrash = (target.id == TRASH_ITEM_ID);
			if (isTrash) {
				hasTrash = true;
			}

			DrawRuleItemCard(vg, tx, ty, ITEM_SIZE, target.id, isThisHovered, isTrash, isThisHovered, target.probability);
		}

		// Ghost Slot
		if (!hasTrash) {
			int tIdx = rule.targets.size();
			int row = tIdx / columns;
			int col = tIdx % columns;
			float tx = targetStartX + col * (ITEM_SIZE + 20 + ITEM_SPACING);
			float ty = ruleY + CARD_PADDING + row * (itemH + ITEM_SPACING);

			bool hoverNewTarget = (m_dragHover.type == HitResult::AddTarget && m_dragHover.ruleIndex == (int)i);

			nvgBeginPath(vg);
			nvgRoundedRect(vg, tx, ty, ITEM_SIZE + 20, itemH, 4);
			if (hoverNewTarget) {
				nvgStrokeColor(vg, accentCol);
				nvgStrokeWidth(vg, 2.0f);
			} else {
				nvgStrokeColor(vg, nvgRGBA(80, 80, 80, 255));
				nvgStrokeWidth(vg, 1.0f);
			}
			nvgStroke(vg);

			if (hoverNewTarget && rule.targets.empty()) {
				DrawTrashIcon(vg, tx + (ITEM_SIZE + 20 - 32.0f) / 2, ty + 8, 32.0f, true);
				nvgFillColor(vg, nvgRGBA(255, 100, 100, 255));
				nvgFontSize(vg, 11.0f);
				nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
				nvgText(vg, tx + (ITEM_SIZE + 20) / 2, ty + 44, "REMOVE", nullptr);
			} else {
				nvgFontSize(vg, 30.0f);
				nvgFillColor(vg, hoverNewTarget ? accentCol : nvgRGBA(100, 100, 100, 255));
				nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
				nvgText(vg, tx + (ITEM_SIZE + 20) / 2, ty + itemH / 2, "+", nullptr);
			}
		}

		// Delete Rule Button
		nvgFillColor(vg, subTextCol);
		nvgFontSize(vg, 16.0f);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		if (m_dragHover.type == HitResult::DeleteRule && m_dragHover.ruleIndex == (int)i) {
			nvgFillColor(vg, nvgRGBA(255, 80, 80, 255));
		}
		nvgText(vg, CARD_MARGIN_X + (width - CARD_MARGIN_X * 2) - 15, ruleY + 15, "X", nullptr);
	}

	// New Rule Area
	int newRuleY = GetRuleY(m_rules.size(), width) + CARD_MARGIN_Y;

	// Determine card style colors
	NVGcolor cardBgTop = nvgRGBA(45, 45, 50, 255);
	NVGcolor cardBgBot = nvgRGBA(40, 40, 45, 255);
	NVGcolor borderColor = nvgRGBA(100, 100, 100, 255);

	// Drop Area
	float dropH = 60.0f;
	float cardX = CARD_MARGIN_X;
	float cardW = width - CARD_MARGIN_X * 2;

	// Fill Card
	NVGpaint bgPaint = nvgLinearGradient(vg, cardX, newRuleY, cardX, newRuleY + dropH, cardBgTop, cardBgBot);
	nvgBeginPath(vg);
	nvgRoundedRect(vg, cardX, newRuleY, cardW, dropH, 4);
	nvgFillPaint(vg, bgPaint);
	nvgFill(vg);

	if (m_dragHover.type == HitResult::NewRule) {
		// Hover Highlight
		nvgStrokeColor(vg, accentCol);
		nvgStrokeWidth(vg, 2.0f);
		nvgFillColor(vg, nvgRGBA(accentCol.r, accentCol.g, accentCol.b, 25)); // ~0.1 alpha
		nvgFill(vg);
	} else {
		// Standard Gray Border (Even if empty)
		nvgStrokeColor(vg, borderColor);
		nvgStrokeWidth(vg, 1.0f);
	}
	nvgStroke(vg);

	nvgFontSize(vg, 14.0f);
	nvgFillColor(vg, subTextCol);
	nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
	nvgText(vg, width / 2.0f, newRuleY + dropH / 2.0f, "Drop Item Here to Add New Rule", nullptr);

	nvgRestore(vg);
}
