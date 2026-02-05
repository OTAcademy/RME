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
static const int CARD_PADDING = 10;
static const int CARD_MARGIN_X = 10;
static const int CARD_MARGIN_Y = 10;
static const int HEADER_HEIGHT = 40; // Space for "Drop New Rule" logic or just top spacing
static const int ITEM_SIZE = 48;
static const int ITEM_SPACING = 8;
static const int ARROW_WIDTH = 40;
static const int SECTION_GAP = 20;
static const int GHOST_SLOT_WIDTH = 80; // Wider for easier drop

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
	} else if (hit.type == RuleBuilderPanel::HitResult::Target) {
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

	m_rowHeight = FromDIP(80); // Base card height, dynamic later?
	m_sourceColWidth = FromDIP(120);

	SetDropTarget(new ItemDropTarget(this));

	Bind(wxEVT_SIZE, &RuleBuilderPanel::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_MOTION, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_LEAVE_WINDOW, &RuleBuilderPanel::OnMouse, this);
	m_pulseTimer.Bind(wxEVT_TIMER, &RuleBuilderPanel::OnPulseTimer, this);

	// Right click to toggle "Delete" status on targets
	Bind(wxEVT_RIGHT_DOWN, [this](wxMouseEvent& evt) {
		HitResult hit = HitTest(evt.GetX(), evt.GetY());
		if (hit.type == HitResult::Target && hit.ruleIndex >= 0 && hit.targetIndex >= 0) {
			auto& rules = m_rules;
			if (hit.ruleIndex < rules.size() && hit.targetIndex < rules[hit.ruleIndex].targets.size()) {
				uint16_t& currentId = rules[hit.ruleIndex].targets[hit.targetIndex].id;
				if (currentId == TRASH_ITEM_ID) {
					// Toggle back to normal? Or just ignore?
					// Implementation choice: do nothing or remove.
					// Let's remove it to give another way to delete.
					rules[hit.ruleIndex].targets.erase(rules[hit.ruleIndex].targets.begin() + hit.targetIndex);
					DistributeProbabilities(hit.ruleIndex);
				} else {
					// Turn into Trash
					currentId = TRASH_ITEM_ID;
				}
				if (m_listener) {
					m_listener->OnRuleChanged();
				}
				Refresh();
			}
		}
	});

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
	// Recalculate scroll height
	// Card Height = PADDING + HEADER + ITEM_SIZE + PADDING (approx 100px)
	int cardHeight = FromDIP(90);
	int totalHeight = m_rules.size() * (cardHeight + CARD_MARGIN_Y) + FromDIP(100); // + New Rule Area
	UpdateScrollbar(totalHeight);
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

// Helper to get card rect
wxRect GetCardRect(int index, int width, int rowHeight) {
	int y = index * (rowHeight + CARD_MARGIN_Y) + CARD_MARGIN_Y;
	return wxRect(CARD_MARGIN_X, y, width - CARD_MARGIN_X * 2, rowHeight);
}

RuleBuilderPanel::HitResult RuleBuilderPanel::HitTest(int x, int y) const {
	int scrollPos = GetScrollPosition();
	int absY = y + scrollPos;
	int width = GetClientSize().x;
	int rowHeight = FromDIP(90);

	// Check Rules
	for (size_t i = 0; i < m_rules.size(); ++i) {
		wxRect card = GetCardRect(i, width, rowHeight);
		if (card.Contains(x, absY)) {
			// Inside Card
			int localX = x - card.x;
			int localY = absY - card.y;

			// Delete Rule Button (Top Right)
			if (localX > card.width - 24 && localY < 24) {
				return { HitResult::DeleteRule, (int)i, -1 };
			}

			// Source Item
			int srcX = CARD_PADDING;
			int contentY = (rowHeight - ITEM_SIZE) / 2;
			if (localX >= srcX && localX <= srcX + ITEM_SIZE && localY >= contentY && localY <= contentY + ITEM_SIZE) {
				return { HitResult::Source, (int)i, -1 };
			}

			// Targets
			int targStartX = srcX + ITEM_SIZE + ARROW_WIDTH;
			// Check each target
			int currentX = targStartX;
			for (size_t t = 0; t < m_rules[i].targets.size(); ++t) {
				if (localX >= currentX && localX <= currentX + ITEM_SIZE && localY >= contentY && localY <= contentY + ITEM_SIZE) {
					// Check for Delete Overlay (X) - simplified, just click the item = Target, let OnMouse handle specifics
					// Actually we need to know if we hit the 'X' overlay or the item.
					// For drag drop, we check item. For click, we might check a sub-rect?
					// Let's assume hitting the target is generic, and we return DeleteTarget if specific region?
					// For now, return Target. We can check specific pixel logic in Mouse handler if needed.
					// Wait, the plan was "X on hover".
					// OnMouse detects click. If click is in top-right of item, maybe DeleteTarget?
					// Let's refine:
					if (localX > currentX + ITEM_SIZE - 16 && localY < contentY + 16) {
						return { HitResult::DeleteTarget, (int)i, (int)t };
					}
					return { HitResult::Target, (int)i, (int)t };
				}
				currentX += ITEM_SIZE + ITEM_SPACING;
			}

			// Add Target Slot [+]
			if (localX >= currentX && localX <= currentX + GHOST_SLOT_WIDTH && localY >= contentY && localY <= contentY + ITEM_SIZE) {
				return { HitResult::AddTarget, (int)i, -1 };
			}

			return { HitResult::None, (int)i, -1 }; // Hit card but nothing specific
		}
	}

	// New Rule Area
	int startNewRuleY = m_rules.size() * (rowHeight + CARD_MARGIN_Y) + CARD_MARGIN_Y * 2;
	if (absY >= startNewRuleY && absY <= startNewRuleY + FromDIP(60)) {
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

void DrawTrashIcon(NVGcontext* vg, float x, float y, float size, bool highlight) {
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

void RuleBuilderPanel::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	int rowHeight = FromDIP(90);
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

	for (size_t i = 0; i < m_rules.size(); ++i) {
		const auto& rule = m_rules[i];
		wxRect r = GetCardRect(i, width, rowHeight);

		// Card BG
		nvgBeginPath(vg);
		nvgRoundedRect(vg, r.x, r.y, r.width, r.height, 4);
		nvgFillColor(vg, nvgRGBA(50, 50, 50, 255));
		nvgFill(vg);
		nvgStrokeColor(vg, nvgRGBA(70, 70, 70, 255));
		nvgStrokeWidth(vg, 1.0f);
		nvgStroke(vg);

		// Source
		int contentY = r.y + (rowHeight - ITEM_SIZE) / 2;
		int srcX = r.x + CARD_PADDING;

		// Label "IF"
		nvgFontSize(vg, 10.0f);
		nvgFillColor(vg, subTextCol);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
		nvgText(vg, srcX, contentY - 4, "IF FOUND", nullptr);

		// Source Item
		if (rule.fromId != 0) {
			int tex = NvgUtils::CreateItemTexture(vg, rule.fromId);
			if (tex > 0) {
				int tw, th;
				nvgImageSize(vg, tex, &tw, &th);
				float scale = (float)ITEM_SIZE / std::max(tw, th);
				float dw = tw * scale;
				float dh = th * scale;
				float dx = srcX + (ITEM_SIZE - dw) / 2;
				float dy = contentY + (ITEM_SIZE - dh) / 2;

				NVGpaint imgPaint = nvgImagePattern(vg, dx, dy, dw, dh, 0.0f, tex, 1.0f);
				nvgBeginPath(vg);
				nvgRect(vg, dx, dy, dw, dh);
				nvgFillPaint(vg, imgPaint);
				nvgFill(vg);
			}
		}

		// Source Hover Glow
		if (m_dragHover.type == HitResult::Source && m_dragHover.ruleIndex == i) {
			nvgBeginPath(vg);
			nvgRoundedRect(vg, srcX - 2, contentY - 2, ITEM_SIZE + 4, ITEM_SIZE + 4, 3);
			nvgStrokeColor(vg, accentCol);
			nvgStrokeWidth(vg, 2.0f);
			nvgStroke(vg);
		}

		// Arrow
		nvgBeginPath(vg);
		float arrowCX = srcX + ITEM_SIZE + ARROW_WIDTH / 2.0f;
		float arrowCY = contentY + ITEM_SIZE / 2.0f;
		nvgMoveTo(vg, arrowCX - 10, arrowCY);
		nvgLineTo(vg, arrowCX + 10, arrowCY);
		nvgLineTo(vg, arrowCX + 5, arrowCY - 5);
		nvgMoveTo(vg, arrowCX + 10, arrowCY);
		nvgLineTo(vg, arrowCX + 5, arrowCY + 5);
		nvgStrokeColor(vg, subTextCol);
		nvgStrokeWidth(vg, 2.0f);
		nvgStroke(vg);

		// Targets
		int currentX = srcX + ITEM_SIZE + ARROW_WIDTH;

		// Label "REPLACE"
		nvgFontSize(vg, 10.0f);
		nvgFillColor(vg, subTextCol);
		nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
		nvgText(vg, currentX, contentY - 4, "REPLACE WITH", nullptr);

		for (size_t t = 0; t < rule.targets.size(); ++t) {
			const auto& target = rule.targets[t];

			// Draw Target Item or Trash
			if (target.id == TRASH_ITEM_ID) {
				DrawTrashIcon(vg, currentX, contentY, ITEM_SIZE, false);
				nvgFillColor(vg, textCol);
				nvgFontSize(vg, 10.0f);
				nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
				nvgText(vg, currentX + ITEM_SIZE / 2, contentY + ITEM_SIZE + 2, "DELETE", nullptr);
			} else if (target.id != 0) {
				int tex = NvgUtils::CreateItemTexture(vg, target.id);
				if (tex > 0) {
					int tw, th;
					nvgImageSize(vg, tex, &tw, &th);
					float scale = (float)ITEM_SIZE / std::max(tw, th);
					float dw = tw * scale;
					float dh = th * scale;
					float dx = currentX + (ITEM_SIZE - dw) / 2;
					float dy = contentY + (ITEM_SIZE - dh) / 2;

					NVGpaint imgPaint = nvgImagePattern(vg, dx, dy, dw, dh, 0.0f, tex, 1.0f);
					nvgBeginPath(vg);
					nvgRect(vg, dx, dy, dw, dh);
					nvgFillPaint(vg, imgPaint);
					nvgFill(vg);
				}
				// Probability
				if (target.probability != 100) {
					nvgFontSize(vg, 10.0f);
					nvgFillColor(vg, textCol);
					nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
					nvgText(vg, currentX + ITEM_SIZE / 2, contentY + ITEM_SIZE + 2, std::format("{}%", target.probability).c_str(), nullptr);
				}
			}

			// Hover X for Delete
			// Show X if hovering the target item at all
			bool isHoveringTarget = (m_dragHover.type == HitResult::Target || m_dragHover.type == HitResult::DeleteTarget)
				&& m_dragHover.ruleIndex == i && m_dragHover.targetIndex == t;

			if (isHoveringTarget) {
				nvgBeginPath(vg);
				nvgCircle(vg, currentX + ITEM_SIZE - 4, contentY + 4, 8);

				// Highlight Red if specifically on the cancel button area (DeleteTarget)
				if (m_dragHover.type == HitResult::DeleteTarget) {
					nvgFillColor(vg, nvgRGBA(255, 80, 80, 255));
				} else {
					nvgFillColor(vg, nvgRGBA(200, 50, 50, 255));
				}
				nvgFill(vg);

				nvgBeginPath(vg); // X mark
				nvgMoveTo(vg, currentX + ITEM_SIZE - 7, contentY + 1);
				nvgLineTo(vg, currentX + ITEM_SIZE - 1, contentY + 7);
				nvgMoveTo(vg, currentX + ITEM_SIZE - 1, contentY + 1);
				nvgLineTo(vg, currentX + ITEM_SIZE - 7, contentY + 7);
				nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
				nvgStrokeWidth(vg, 1.5f);
				nvgStroke(vg);
			}

			// Hover Glow for Target Drop (Only if NOT on delete button to avoid confusion?)
			// Actually keep glow for slot, X is overlay.
			if (isHoveringTarget && m_dragHover.type != HitResult::DeleteTarget) {
				nvgBeginPath(vg);
				nvgRoundedRect(vg, currentX - 2, contentY - 2, ITEM_SIZE + 4, ITEM_SIZE + 4, 3);
				nvgStrokeColor(vg, accentCol);
				nvgStrokeWidth(vg, 2.0f);
				nvgStroke(vg);
			}

			currentX += ITEM_SIZE + ITEM_SPACING;
		}

		// Add Target Ghost Slot
		nvgBeginPath(vg);
		nvgRoundedRect(vg, currentX, contentY, GHOST_SLOT_WIDTH, ITEM_SIZE, 4);
		nvgStrokeColor(vg, nvgRGBA(80, 80, 80, 255));
		nvgStrokeWidth(vg, 1.0f);
		// Dashed? NanoVG support dashed via patterns but simple stroke is ok.
		nvgStroke(vg);

		nvgFontSize(vg, 24.0f);
		nvgFillColor(vg, nvgRGBA(100, 100, 100, 255));
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, currentX + GHOST_SLOT_WIDTH / 2, contentY + ITEM_SIZE / 2, "+", nullptr);

		if (m_dragHover.type == HitResult::AddTarget && m_dragHover.ruleIndex == i) {
			nvgStrokeColor(vg, accentCol);
			nvgStrokeWidth(vg, 2.0f);
			nvgStroke(vg);
		}

		// Delete Rule Button (Top Right)
		nvgFontSize(vg, 14.0f);
		nvgFillColor(vg, nvgRGBA(150, 150, 150, 255));
		nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_TOP);

		if (m_dragHover.type == HitResult::DeleteRule && m_dragHover.ruleIndex == i) {
			nvgFillColor(vg, nvgRGBA(255, 80, 80, 255));
		}
		nvgText(vg, r.x + r.width - 8, r.y + 4, "x", nullptr);
	}

	// New Rule Area
	int newRuleY = m_rules.size() * (rowHeight + CARD_MARGIN_Y) + CARD_MARGIN_Y * 2;
	if (m_rules.empty()) {
		// Pulse Outline Color
		NVGcolor pulseCol = nvgRGBA(accentCol.r, accentCol.g, accentCol.b, 100 + (int)(155 * pulse));

		nvgFontSize(vg, 18.0f);
		nvgFontFace(vg, "sans-bold");
		nvgFillColor(vg, pulseCol);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, width / 2.0f, newRuleY + 40, "No Rules Yet", nullptr);

		nvgFontSize(vg, 14.0f);
		nvgFontFace(vg, "sans");
		nvgFillColor(vg, subTextCol);
		nvgText(vg, width / 2.0f, newRuleY + 70, "Drag an item here to start building rules.", nullptr);

		// Pulsing Drop Area Border
		float dropH = 100.0f;
		nvgBeginPath(vg);
		nvgRoundedRect(vg, CARD_MARGIN_X, newRuleY, width - CARD_MARGIN_X * 2, dropH, 4);
		nvgStrokeColor(vg, pulseCol);
		nvgStrokeWidth(vg, 2.0f + 2.0f * pulse);
		nvgStroke(vg);

	} else {
		// Drop Area
		float dropH = 60.0f;
		nvgBeginPath(vg);
		nvgRoundedRect(vg, CARD_MARGIN_X, newRuleY, width - CARD_MARGIN_X * 2, dropH, 4);

		if (m_dragHover.type == HitResult::NewRule) {
			nvgStrokeColor(vg, accentCol);
			nvgStrokeWidth(vg, 2.0f);
			nvgFillColor(vg, nvgRGBA(accentCol.r, accentCol.g, accentCol.b, 0.1));
			nvgFill(vg);
		} else {
			nvgStrokeColor(vg, nvgRGBA(70, 70, 70, 255));
			nvgStrokeWidth(vg, 1.0f);
		}
		nvgStroke(vg);

		nvgFontSize(vg, 14.0f);
		nvgFillColor(vg, subTextCol);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, width / 2.0f, newRuleY + dropH / 2.0f, "Drop Item Here to Add New Rule", nullptr);
	}

	nvgRestore(vg);
}
