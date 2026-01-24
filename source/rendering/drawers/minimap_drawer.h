//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_DRAWERS_MINIMAP_DRAWER_H_
#define RME_RENDERING_DRAWERS_MINIMAP_DRAWER_H_

#include "rendering/drawers/minimap_renderer.h"
#include <memory>

class Editor;
class MapCanvas;
class PrimitiveRenderer;

class MinimapDrawer {
public:
	MinimapDrawer();
	~MinimapDrawer();

	void Draw(wxDC& dc, const wxSize& size, Editor& editor, MapCanvas* canvas);

	void ScreenToMap(int screen_x, int screen_y, int& map_x, int& map_y);

	int GetLastStartX() const {
		return last_start_x;
	}
	int GetLastStartY() const {
		return last_start_y;
	}

private:
	std::unique_ptr<MinimapRenderer> renderer;
	std::unique_ptr<PrimitiveRenderer> primitive_renderer;
	int last_start_x;
	int last_start_y;
};

#endif
