#ifndef RME_UI_REPLACE_TOOL_CARD_PANEL_H_
#define RME_UI_REPLACE_TOOL_CARD_PANEL_H_

#include <wx/panel.h>
#include <wx/string.h>
#include <wx/sizer.h>
#include <wx/event.h>

class CardPanel : public wxPanel {
public:
	CardPanel(wxWindow* parent, wxWindowID id = wxID_ANY);

	// Optional: Set specific card style properties if needed
	void SetShowFooter(bool show);
	void SetTitle(const wxString& title);
	wxSizer* GetContentSizer() const {
		return m_contentSizer;
	}
	wxSizer* GetFooterSizer() const {
		return m_footerSizer;
	}

	static const int HEADER_HEIGHT = 32;
	static const int FOOTER_HEIGHT = 42;

protected:
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);

private:
	bool m_showFooter = false;
	wxString m_title;

	wxBoxSizer* m_mainSizer = nullptr;
	wxBoxSizer* m_contentSizer = nullptr;
	wxBoxSizer* m_footerSizer = nullptr;
};

#endif
