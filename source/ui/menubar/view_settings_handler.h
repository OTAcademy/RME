#ifndef RME_UI_MENUBAR_VIEW_SETTINGS_HANDLER_H_
#define RME_UI_MENUBAR_VIEW_SETTINGS_HANDLER_H_

#include <wx/wx.h>

class MainMenuBar;

class ViewSettingsHandler {
public:
	ViewSettingsHandler(MainMenuBar* menuBar);

	void LoadValues();
	void OnChangeViewSettings(wxCommandEvent& event);
	void OnToolbars(wxCommandEvent& event);
	void OnToggleAutomagic(wxCommandEvent& event);
	void OnSelectionTypeChange(wxCommandEvent& event);

private:
	MainMenuBar* menuBar;
};

#endif
