#ifndef RME_UI_MENUBAR_MAP_ACTIONS_HANDLER_H_
#define RME_UI_MENUBAR_MAP_ACTIONS_HANDLER_H_

#include <wx/wx.h>

class MainFrame;

class MapActionsHandler {
public:
	MapActionsHandler(MainFrame* frame);

	void OnMapRemoveItems(wxCommandEvent& event);
	void OnMapRemoveCorpses(wxCommandEvent& event);
	void OnMapRemoveUnreachable(wxCommandEvent& event);
	void OnClearHouseTiles(wxCommandEvent& event);
	void OnClearModifiedState(wxCommandEvent& event);
	void OnMapCleanHouseItems(wxCommandEvent& event);
	void OnBorderizeSelection(wxCommandEvent& event);
	void OnBorderizeMap(wxCommandEvent& event);
	void OnRandomizeSelection(wxCommandEvent& event);
	void OnRandomizeMap(wxCommandEvent& event);
	void OnMapCleanup(wxCommandEvent& event);

private:
	MainFrame* frame;
};

#endif
