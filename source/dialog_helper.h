//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DIALOG_HELPER_H_
#define RME_DIALOG_HELPER_H_

#include <wx/wx.h>

class Editor;
class Tile;

class DialogHelper {
public:
	static void OpenProperties(Editor& editor, Tile* tile);
};

#endif
