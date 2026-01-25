#ifndef RME_UI_MENUBAR_PALETTE_MENU_HANDLER_H
#define RME_UI_MENUBAR_PALETTE_MENU_HANDLER_H

#include <wx/wx.h>

class MainFrame;
class MainMenuBar;

class PaletteMenuHandler : public wxEvtHandler {
public:
	PaletteMenuHandler(MainFrame* frame, MainMenuBar* menubar);
	~PaletteMenuHandler();

	void OnNewPalette(wxCommandEvent& event);

	void OnSelectTerrainPalette(wxCommandEvent& event);
	void OnSelectDoodadPalette(wxCommandEvent& event);
	void OnSelectItemPalette(wxCommandEvent& event);
	void OnSelectCollectionPalette(wxCommandEvent& event);
	void OnSelectHousePalette(wxCommandEvent& event);
	void OnSelectCreaturePalette(wxCommandEvent& event);
	void OnSelectWaypointPalette(wxCommandEvent& event);
	void OnSelectRawPalette(wxCommandEvent& event);

protected:
	MainFrame* frame;
	MainMenuBar* menubar;
};

#endif
