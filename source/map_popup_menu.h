//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_MAP_POPUP_MENU_H_
#define RME_MAP_POPUP_MENU_H_

#include <wx/wx.h>

class Editor;

// Right-click popup menu
class MapPopupMenu : public wxMenu {
public:
	MapPopupMenu(Editor& editor);
	virtual ~MapPopupMenu();

	void Update();

protected:
	Editor& editor;
};

#endif
