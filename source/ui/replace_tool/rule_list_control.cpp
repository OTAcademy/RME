#include "ui/replace_tool/rule_list_control.h"
#include "ui/theme.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

RuleListControl::RuleListControl(wxWindow* parent, Listener* listener) : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxNO_BORDER | wxWANTS_CHARS),
																		 m_listener(listener) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_itemHeight = FromDIP(56);

	Bind(wxEVT_PAINT, &RuleListControl::OnPaint, this);
	Bind(wxEVT_SIZE, &RuleListControl::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &RuleListControl::OnMouse, this);
	Bind(wxEVT_MOTION, &RuleListControl::OnMouse, this);
	Bind(wxEVT_LEAVE_WINDOW, &RuleListControl::OnMouse, this);
	Bind(wxEVT_CONTEXT_MENU, &RuleListControl::OnContextMenu, this);

	// Bind scroll events
	auto scrollHandler = [this](wxScrollWinEvent& event) { Refresh(); event.Skip(); };
	Bind(wxEVT_SCROLLWIN_THUMBTRACK, scrollHandler);
	Bind(wxEVT_SCROLLWIN_THUMBRELEASE, scrollHandler);
	Bind(wxEVT_SCROLLWIN_LINEDOWN, scrollHandler);
	Bind(wxEVT_SCROLLWIN_LINEUP, scrollHandler);

	Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) { });
}

void RuleListControl::SetRuleSets(const std::vector<std::string>& ruleSetNames) {
	m_ruleSetNames = ruleSetNames;
	m_selectedIndex = -1;
	RefreshVirtualSize();
	Refresh();
}

void RuleListControl::RefreshVirtualSize() {
	wxSize clientSize = GetClientSize();
	int totalHeight = m_ruleSetNames.size() * m_itemHeight;
	SetScrollbar(wxVERTICAL, GetScrollPos(wxVERTICAL), clientSize.y, totalHeight);
}

wxSize RuleListControl::DoGetBestClientSize() const {
	return wxSize(FromDIP(150), FromDIP(200));
}

void RuleListControl::OnSize(wxSizeEvent& event) {
	RefreshVirtualSize();
	event.Skip();
}

void RuleListControl::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	dc.Clear();

	std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));
	if (!gc) {
		return;
	}

	wxSize clientSize = GetClientSize();
	int scrollPos = GetScrollPos(wxVERTICAL);

	// Larger height for card look, maybe 56 or 64. I'll use whatever is set for now but subtract padding for drawing
	int startIdx = scrollPos / m_itemHeight;
	int endIdx = std::min((int)m_ruleSetNames.size() - 1, (scrollPos + clientSize.y) / m_itemHeight);

	const double padding = 4.0;
	const double radius = 4.0;

	// Card Colors
	wxColour cardBase = wxColour(50, 50, 55);
	wxColour cardBaseHover = wxColour(60, 60, 65);
	wxColour cardBaseSelected = wxColour(70, 70, 80); // Or Accent

	for (int i = startIdx; i <= endIdx; ++i) {
		wxRect rect(padding, i * m_itemHeight - scrollPos + padding, clientSize.x - 2 * padding, m_itemHeight - 2 * padding);

		// Determine State
		bool isSelected = (i == m_selectedIndex);
		bool isHovered = (i == m_hoveredIndex);

		wxColour fillColor = cardBase;
		if (isSelected) {
			fillColor = Theme::Get(Theme::Role::Accent);
		} else if (isHovered) {
			fillColor = cardBaseHover;
		}

		// Draw Card Path
		wxGraphicsPath path = gc->CreatePath();
		path.AddRoundedRectangle(rect.x, rect.y, rect.width, rect.height, radius);

		// Shadow (Simple offset)
		if (!isSelected) {
			gc->SetBrush(wxBrush(wxColour(0, 0, 0, 50)));
			gc->SetPen(*wxTRANSPARENT_PEN);
			gc->DrawRoundedRectangle(rect.x + 2, rect.y + 2, rect.width, rect.height, radius);
		}

		// Fill
		gc->SetBrush(wxBrush(fillColor));
		if (isSelected) {
			// Add a border for selection?
			gc->SetPen(wxPen(*wxWHITE, 1));
		} else {
			gc->SetPen(wxPen(wxColour(80, 80, 80), 1));
		}
		gc->FillPath(path);
		gc->StrokePath(path);

		// Text
		dc.SetFont(Theme::GetFont(10, isSelected));
		if (isSelected) {
			dc.SetTextForeground(*wxWHITE);
		} else {
			dc.SetTextForeground(Theme::Get(Theme::Role::Text));
		}

		wxString label = m_ruleSetNames[i];
		wxSize textSize = dc.GetTextExtent(label);

		// Clip to card ?? Or just draw
		dc.DrawText(label, rect.x + 10, rect.y + (rect.height - textSize.y) / 2);
	}
}

void RuleListControl::OnMouse(wxMouseEvent& event) {
	int oldHover = m_hoveredIndex;

	if (event.GetEventType() == wxEVT_LEAVE_WINDOW) {
		m_hoveredIndex = -1;
	} else {
		int scrollPos = GetScrollPos(wxVERTICAL);
		int y = event.GetY() + scrollPos;
		int idx = y / m_itemHeight;

		if (idx >= 0 && idx < (int)m_ruleSetNames.size()) {
			m_hoveredIndex = idx;
		} else {
			m_hoveredIndex = -1;
		}

		if (event.LeftDown() && m_hoveredIndex != -1) {
			m_selectedIndex = m_hoveredIndex;
			if (m_listener) {
				m_listener->OnRuleSelected(RuleManager::Get().LoadRuleSet(m_ruleSetNames[m_selectedIndex]));
			}
		}
	}

	if (oldHover != m_hoveredIndex || event.LeftDown()) {
		Refresh();
	}
}
void RuleListControl::OnContextMenu(wxContextMenuEvent& event) {
	int menuIdx = m_hoveredIndex;
	if (menuIdx == -1) {
		return;
	}

	wxMenu menu;
	menu.Append(wxID_EDIT, "Edit Name");
	menu.Append(wxID_DELETE, "Delete");

	menu.Bind(wxEVT_MENU, [this, menuIdx](wxCommandEvent& e) {
		if (menuIdx < 0 || menuIdx >= (int)m_ruleSetNames.size()) {
			return;
		}

		std::string name = m_ruleSetNames[menuIdx];

		if (e.GetId() == wxID_EDIT) {
			wxString newName = wxGetTextFromUser("Enter new name for rule set:", "Edit Name", name);
			if (!newName.IsEmpty() && newName.ToStdString() != name) {
				if (m_listener) {
					m_listener->OnRuleRenamed(name, newName.ToStdString());
				}
			}
		} else if (e.GetId() == wxID_DELETE) {
			wxMessageDialog dlg(this, "Are you sure you want to delete rule set '" + name + "'?", "Delete Rule Set", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
			if (dlg.ShowModal() == wxID_YES) {
				if (m_listener) {
					m_listener->OnRuleDeleted(name);
				}
			}
		}
	});

	PopupMenu(&menu);
}
