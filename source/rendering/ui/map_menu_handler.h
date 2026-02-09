//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_UI_MAP_MENU_HANDLER_H_
#define RME_RENDERING_UI_MAP_MENU_HANDLER_H_

#include "app/main.h"
#include <wx/event.h>

class MapCanvas;
class Editor;

class MapMenuHandler {
public:
	MapMenuHandler(MapCanvas* canvas, Editor& editor);
	~MapMenuHandler() = default;

	void BindEvents();

	void OnCut(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);
	void OnCopyPosition(wxCommandEvent& event);
	void OnCopyServerId(wxCommandEvent& event);
	void OnCopyClientId(wxCommandEvent& event);
	void OnCopyName(wxCommandEvent& event);
	void OnBrowseTile(wxCommandEvent& event);
	void OnPaste(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);

	void OnGotoDestination(wxCommandEvent& event);
	void OnRotateItem(wxCommandEvent& event);
	void OnSwitchDoor(wxCommandEvent& event);

	void OnSelectRAWBrush(wxCommandEvent& event);
	void OnSelectGroundBrush(wxCommandEvent& event);
	void OnSelectDoodadBrush(wxCommandEvent& event);
	void OnSelectDoorBrush(wxCommandEvent& event);
	void OnSelectWallBrush(wxCommandEvent& event);
	void OnSelectCarpetBrush(wxCommandEvent& event);
	void OnSelectTableBrush(wxCommandEvent& event);
	void OnSelectCreatureBrush(wxCommandEvent& event);
	void OnSelectSpawnBrush(wxCommandEvent& event);
	void OnSelectHouseBrush(wxCommandEvent& event);
	void OnSelectCollectionBrush(wxCommandEvent& event);
	void OnSelectMoveTo(wxCommandEvent& event);

	void OnProperties(wxCommandEvent& event);
	void OnAdvancedReplace(wxCommandEvent& event);

private:
	MapCanvas* canvas;
	Editor& editor;
};

#endif
