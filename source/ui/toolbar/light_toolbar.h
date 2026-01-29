//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_LIGHT_TOOLBAR_H_
#define RME_UI_TOOLBAR_LIGHT_TOOLBAR_H_

#include <wx/wx.h>
#include <wx/aui/auibar.h>
#include <wx/slider.h>

class LightToolBar : public wxEvtHandler {
public:
	LightToolBar(wxWindow* parent);
	~LightToolBar();

	wxAuiToolBar* GetToolbar() const {
		return toolbar;
	}

	void OnLightSlider(wxCommandEvent& event);
	void OnAmbientLightSlider(wxCommandEvent& event);
	void OnToggleLight(wxCommandEvent& event);

	static const wxString PANE_NAME;

private:
	wxAuiToolBar* toolbar;
	wxSlider* light_slider;
	wxSlider* ambient_slider;
};

#endif // RME_UI_TOOLBAR_LIGHT_TOOLBAR_H_
