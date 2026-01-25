#ifndef RME_UI_MENUBAR_NAVIGATION_MENU_HANDLER_H
#define RME_UI_MENUBAR_NAVIGATION_MENU_HANDLER_H

#include <wx/wx.h>

class MainFrame;
class MainMenuBar;

class NavigationMenuHandler : public wxEvtHandler {
public:
	NavigationMenuHandler(MainFrame* frame, MainMenuBar* menubar);
	~NavigationMenuHandler();

	void OnZoomIn(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnZoomNormal(wxCommandEvent& event);

	void OnGotoPreviousPosition(wxCommandEvent& event);
	void OnGotoPosition(wxCommandEvent& event);
	void OnJumpToBrush(wxCommandEvent& event);
	void OnJumpToItemBrush(wxCommandEvent& event);

	void OnChangeFloor(wxCommandEvent& event);

protected:
	MainFrame* frame;
	MainMenuBar* menubar;
};

#endif
