//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_UI_SELECTION_CONTROLLER_H_
#define RME_RENDERING_UI_SELECTION_CONTROLLER_H_

#include <wx/event.h>
#include <vector>
#include "position.h"

class MapCanvas;
class Editor;
class Tile;
class SelectionThread;

class SelectionController {
public:
	SelectionController(MapCanvas* canvas, Editor& editor);
	~SelectionController();

	void HandleClick(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandleDrag(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandleRelease(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandlePropertiesClick(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandlePropertiesRelease(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down);
	void HandleDoubleClick(const Position& mouse_map_pos);

	bool IsDragging() const {
		return dragging;
	}
	bool IsBoundboxSelection() const {
		return boundbox_selection;
	}

	void StartDragging(const Position& start_pos) {
		dragging = true;
		drag_start_pos = start_pos;
	}

	Position GetDragStartPosition() const {
		return drag_start_pos;
	}

	void Reset() {
		dragging = false;
		boundbox_selection = false;
	}

private:
	void ExecuteBoundboxSelection(const Position& start_pos, const Position& end_pos, int floor);

	MapCanvas* canvas;
	Editor& editor;

	bool dragging;
	bool boundbox_selection;

	Position drag_start_pos;
};

#endif
