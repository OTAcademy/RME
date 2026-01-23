//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_DRAWERS_MINIMAP_DRAWER_H_
#define RME_RENDERING_DRAWERS_MINIMAP_DRAWER_H_

#include <wx/wx.h>

class Editor;
class MapCanvas;

class MinimapDrawer {
public:
	MinimapDrawer();
	~MinimapDrawer();

	void Draw(wxDC& dc, const wxSize& size, Editor& editor, MapCanvas* canvas);

	int GetLastStartX() const {
		return last_start_x;
	}
	int GetLastStartY() const {
		return last_start_y;
	}

private:
	wxPen* pens[256];
	int last_start_x;
	int last_start_y;
};

#endif
