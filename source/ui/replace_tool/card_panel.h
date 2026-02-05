#ifndef RME_UI_REPLACE_TOOL_CARD_PANEL_H_
#define RME_UI_REPLACE_TOOL_CARD_PANEL_H_

#include <wx/panel.h>
#include <wx/string.h>

class CardPanel : public wxPanel {
public:
	CardPanel(wxWindow* parent, wxWindowID id = wxID_ANY);

	// Optional: Set specific card style properties if needed
	void SetIsActive(bool active);
	void SetTitle(const wxString& title);

	static const int HEADER_HEIGHT = 32;

protected:
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

private:
	bool m_isActive = false;
	wxString m_title;
};

#endif
