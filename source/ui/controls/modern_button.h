#ifndef MODERN_BUTTON_H
#define MODERN_BUTTON_H

#include <wx/wx.h>
#include <wx/dcbuffer.h>

class ModernButton : public wxControl {
public:
	ModernButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);

	// Mandatory overrides
	wxSize DoGetBestClientSize() const override;
	void DoSetSizeHints(int minW, int minH, int maxW, int maxH, int incW, int incH) override;

	// Rendering
	void OnPaint(wxPaintEvent& evt);
	void OnMouse(wxMouseEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt);

	// Animation state
	float m_hoverAlpha = 0.0f; // 0.0-1.0
	float m_targetHover = 0.0f;
	wxTimer m_animTimer;
	void OnTimer(wxTimerEvent& evt);

private:
	wxDECLARE_EVENT_TABLE();
};

#endif
