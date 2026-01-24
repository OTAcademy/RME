//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_UI_DRAWING_CONTROLLER_H_
#define RME_RENDERING_UI_DRAWING_CONTROLLER_H_

#include <wx/event.h>
#include "map/position.h"

class MapCanvas;
class Editor;

class DrawingController {
public:
	DrawingController(MapCanvas* canvas, Editor& editor);
	~DrawingController();

	void HandleClick(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandleDrag(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandleRelease(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandleWheel(int rotation, bool alt_down, bool ctrl_down);

	bool IsDrawing() const {
		return drawing;
	}
	bool IsDraggingDraw() const {
		return dragging_draw;
	}
	bool IsReplaceDragging() const {
		return replace_dragging;
	}

	void Reset() {
		drawing = false;
		dragging_draw = false;
		replace_dragging = false;
	}

private:
	MapCanvas* canvas;
	Editor& editor;

	bool drawing;
	bool dragging_draw;
	bool replace_dragging;

	Position last_draw_pos;
};

#endif
