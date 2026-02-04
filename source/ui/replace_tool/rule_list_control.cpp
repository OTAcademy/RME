#include "ui/replace_tool/rule_list_control.h"
#include "ui/theme.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

RuleListControl::RuleListControl(wxWindow* parent, Listener* listener) : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxNO_BORDER | wxWANTS_CHARS),
																		 m_listener(listener) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_itemHeight = FromDIP(48);

	Bind(wxEVT_PAINT, &RuleListControl::OnPaint, this);
	Bind(wxEVT_SIZE, &RuleListControl::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &RuleListControl::OnMouse, this);
	Bind(wxEVT_MOTION, &RuleListControl::OnMouse, this);
	Bind(wxEVT_LEAVE_WINDOW, &RuleListControl::OnMouse, this);

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

	wxSize clientSize = GetClientSize();
	int scrollPos = GetScrollPos(wxVERTICAL);

	int startIdx = scrollPos / m_itemHeight;
	int endIdx = std::min((int)m_ruleSetNames.size() - 1, (scrollPos + clientSize.y) / m_itemHeight);

	for (int i = startIdx; i <= endIdx; ++i) {
		wxRect rect(0, i * m_itemHeight - scrollPos, clientSize.x, m_itemHeight);

		// Hover/Selection Background
		if (i == m_selectedIndex) {
			dc.SetBrush(wxBrush(Theme::Get(Theme::Role::Accent)));
		} else if (i == m_hoveredIndex) {
			wxColour bg = Theme::Get(Theme::Role::Surface);
			// Slightly lighter for hover
			dc.SetBrush(wxBrush(bg.ChangeLightness(110)));
		} else {
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
		}

		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(rect);

		// Text
		dc.SetFont(Theme::GetFont(10, i == m_selectedIndex));
		dc.SetTextForeground(Theme::Get(Theme::Role::Text));

		wxString label = m_ruleSetNames[i];
		wxSize textSize = dc.GetTextExtent(label);
		dc.DrawText(label, Theme::Grid(2), rect.y + (m_itemHeight - textSize.y) / 2);
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
