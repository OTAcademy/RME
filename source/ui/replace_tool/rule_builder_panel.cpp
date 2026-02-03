#include "ui/replace_tool/rule_builder_panel.h"
#include "ui/theme.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/msgdlg.h>

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
		m_panel->m_rules.push_back(newRule);
		m_panel->m_listener->OnRuleChanged();
		m_panel->Refresh();
		return true;
	}

	return false;
}

wxDragResult RuleBuilderPanel::ItemDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
	RuleBuilderPanel::HitResult hit = m_panel->HitTest(x, y);
	m_panel->m_dragHover = hit;
	m_panel->Refresh(); // Redraw for feedback
	return def;
}

void RuleBuilderPanel::ItemDropTarget::OnLeave() {
	m_panel->m_dragHover = { RuleBuilderPanel::HitResult::None, -1, -1 };
	m_panel->Refresh();
}

// ----------------------------------------------------------------------------
// RuleBuilderPanel
// ----------------------------------------------------------------------------

RuleBuilderPanel::RuleBuilderPanel(wxWindow* parent, Listener* listener) : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
																		   m_listener(listener) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetBackgroundColour(Theme::Get(Theme::Role::Surface));

	m_rowHeight = FromDIP(48); // 32px + padding
	// User requested layout: Sprite | Original ID | Replacement ID | Replacements List | Similar Sprites?
	// Simplified to: Source (Sprite+ID) | Replacements (List)

	m_sourceColWidth = FromDIP(80);

	SetDropTarget(new ItemDropTarget(this));

	Bind(wxEVT_PAINT, &RuleBuilderPanel::OnPaint, this);
	Bind(wxEVT_SIZE, &RuleBuilderPanel::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, [this](wxMouseEvent& evt) {
		HitResult hit = HitTest(evt.GetX(), evt.GetY());
		if (hit.type == HitResult::Source && hit.ruleIndex != -1) {
			// Remove entire rule
			if (hit.ruleIndex < m_rules.size()) {
				m_rules.erase(m_rules.begin() + hit.ruleIndex);
				if (m_listener) {
					m_listener->OnRuleChanged();
				}
				Refresh();
			}
		} else if (hit.type == HitResult::Target && hit.ruleIndex != -1 && hit.targetIndex != -1) {
			// Remove specific target
			auto& targets = m_rules[hit.ruleIndex].targets;
			if (hit.targetIndex < targets.size()) {
				targets.erase(targets.begin() + hit.targetIndex);
				DistributeProbabilities(hit.ruleIndex);
				if (m_listener) {
					m_listener->OnRuleChanged();
				}
				Refresh();
			}
		}
	});

	m_dragHover = { HitResult::None, -1, -1 };
}

RuleBuilderPanel::~RuleBuilderPanel() { }

void RuleBuilderPanel::SetRules(const std::vector<ReplacementRule>& rules) {
	m_rules = rules;
	Refresh();
}

std::vector<ReplacementRule> RuleBuilderPanel::GetRules() const {
	return m_rules;
}

void RuleBuilderPanel::Clear() {
	m_rules.clear();
	Refresh();
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
	return wxSize(FromDIP(400), FromDIP(300));
}

void RuleBuilderPanel::OnSize(wxSizeEvent& event) {
	event.Skip();
}

RuleBuilderPanel::HitResult RuleBuilderPanel::HitTest(int x, int y) const {
	int headerHeight = FromDIP(24);
	if (y < headerHeight) {
		return { HitResult::None, -1, -1 };
	}

	int relY = y - headerHeight;
	int rowIndex = relY / m_rowHeight;

	if (rowIndex < (int)m_rules.size()) {
		if (x < m_sourceColWidth) {
			return { HitResult::Source, rowIndex, -1 };
		} else {
			// Check specific target
			int targetX = x - m_sourceColWidth;
			int itemWithPad = FromDIP(36); // 32 item + 4 pad
			int targetIdx = targetX / itemWithPad;

			if (targetIdx < m_rules[rowIndex].targets.size()) {
				return { HitResult::Target, rowIndex, targetIdx };
			}
			// Still technically in target area (for adding new target)
			return { HitResult::Target, rowIndex, -1 };
		}
	}

	// Below all rows
	return { HitResult::NewRule, -1, -1 };
}

void RuleBuilderPanel::OnMouse(wxMouseEvent& event) {
	event.Skip();
}

void RuleBuilderPanel::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	dc.Clear();

	wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
	if (!gc) {
		return;
	}

	gc->SetAntialiasMode(wxANTIALIAS_DEFAULT);

	wxColour textCol = Theme::Get(Theme::Role::Text);
	wxColour subTextCol = Theme::Get(Theme::Role::TextSubtle);
	wxColour accent = Theme::Get(Theme::Role::Accent);
	wxColour borderCol = wxColour(60, 60, 60);

	int w, h;
	GetClientSize(&w, &h);

	int headerHeight = FromDIP(24);

	// Header
	dc.SetFont(Theme::GetFont(8, true));
	dc.SetTextForeground(subTextCol);
	dc.DrawText("Original", Theme::Grid(1), Theme::Grid(1));
	dc.DrawText("Replacements", m_sourceColWidth + Theme::Grid(1), Theme::Grid(1));

	dc.SetPen(wxPen(borderCol));
	dc.DrawLine(0, headerHeight, w, headerHeight);

	int y = headerHeight;

	// Draw Rules
	for (size_t i = 0; i < m_rules.size(); ++i) {
		const ReplacementRule& rule = m_rules[i];

		// Alternating row background?
		if (i % 2 == 1) {
			dc.SetBrush(wxBrush(wxColour(40, 40, 40)));
			dc.SetPen(*wxTRANSPARENT_PEN);
			dc.DrawRectangle(0, y, w, m_rowHeight);
		}

		// Source Item
		if (rule.fromId != 0) {
			Sprite* s = g_gui.gfx.getSprite(rule.fromId);
			if (s) {
				s->DrawTo(&dc, SPRITE_SIZE_32x32, Theme::Grid(1), y + (m_rowHeight - 32) / 2);
			}
			// ID text
			dc.SetFont(Theme::GetFont(8));
			dc.SetTextForeground(textCol);
			dc.DrawText(wxString::Format("%d", rule.fromId), Theme::Grid(1) + 32 + 5, y + (m_rowHeight - 12) / 2);
		}

		// Highlight Source Drop
		if (m_dragHover.type == HitResult::Source && m_dragHover.ruleIndex == (int)i) {
			gc->SetPen(wxPen(accent, 2));
			gc->SetBrush(*wxTRANSPARENT_BRUSH);
			gc->DrawRectangle(1, y + 1, m_sourceColWidth - 2, m_rowHeight - 2);
		}

		// Targets
		int x = m_sourceColWidth;
		int itemWithPad = FromDIP(36);

		for (size_t t = 0; t < rule.targets.size(); ++t) {
			const auto& target = rule.targets[t];

			// Draw Target Sprite
			Sprite* s = g_gui.gfx.getSprite(target.id);
			if (s) {
				s->DrawTo(&dc, SPRITE_SIZE_32x32, x, y + (m_rowHeight - 32) / 2);
			}

			// Probability overlay?
			if (target.probability != 100) {
				wxString prob = wxString::Format("%d", target.probability);
				wxFont smallFont = Theme::GetFont(7);
				dc.SetFont(smallFont);
				wxSize sz = dc.GetTextExtent(prob);
				dc.SetTextForeground(*wxWHITE);
				// Make a tiny box for readability
				dc.SetBrush(wxBrush(wxColour(0, 0, 0, 180)));
				dc.SetPen(*wxTRANSPARENT_PEN);
				dc.DrawRectangle(x + 32 - sz.x, y + (m_rowHeight - 32) / 2 + 32 - sz.y, sz.x, sz.y);
				dc.DrawText(prob, x + 32 - sz.x, y + (m_rowHeight - 32) / 2 + 32 - sz.y);
			}

			x += itemWithPad;
		}

		// Highlight Target Drop for this row
		if (m_dragHover.type == HitResult::Target && m_dragHover.ruleIndex == (int)i) {
			gc->SetPen(wxPen(accent, 2));
			gc->SetBrush(*wxTRANSPARENT_BRUSH);
			// Draw rect around the "add new target" slot at end of list
			gc->DrawRectangle(x, y + (m_rowHeight - 32) / 2, 32, 32);
		}

		// Divider
		dc.SetPen(wxPen(borderCol));
		dc.DrawLine(0, y + m_rowHeight, w, y + m_rowHeight);

		y += m_rowHeight;
	}

	// "New Rule" Drop Area
	// If dragging, show a placeholder row if hovering below
	if (m_dragHover.type == HitResult::NewRule) {
		dc.SetPen(wxPen(accent, 2, wxPENSTYLE_DOT)); // Dotted line?
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.DrawRectangle(Theme::Grid(1), y + Theme::Grid(1), w - Theme::Grid(2), m_rowHeight);

		dc.SetFont(Theme::GetFont(10));
		dc.SetTextForeground(accent);
		dc.DrawText("Drop to Add New Rule", Theme::Grid(4), y + Theme::Grid(1) + 10);
	} else if (m_rules.empty()) {
		// Empty state
		dc.SetFont(Theme::GetFont(10));
		dc.SetTextForeground(subTextCol);
		wxSize sz = dc.GetTextExtent("Drag items here to create rules");
		dc.DrawText("Drag items here to create rules", (w - sz.x) / 2, y + 20);
	}

	delete gc;
}
