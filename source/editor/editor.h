//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_EDITOR_H
#define RME_EDITOR_H

#include "game/item.h"
#include "map/tile.h"
#include "io/iomap.h"
#include "map/map.h"

#include "editor/action.h"
#include "editor/selection.h"
#include "editor/operations/selection_operations.h"
#include "map/operations/map_processor.h"
#include "editor/persistence/editor_persistence.h"

class BaseMap;
class CopyBuffer;
class LiveClient;
class LiveServer;
class LiveSocket;

#include "live/live_manager.h"

#include <functional>

class Editor {
public:
	Editor(CopyBuffer& copybuffer, const MapVersion& version, std::unique_ptr<LiveClient> client);
	Editor(CopyBuffer& copybuffer, const MapVersion& version, const FileName& fn);
	Editor(CopyBuffer& copybuffer, const MapVersion& version);
	~Editor();

	// Live Manager
	LiveManager live_manager;

public:
	// Public members
	ActionQueue* actionQueue;
	Selection selection;
	CopyBuffer& copybuffer;
	GroundBrush* replace_brush;
	Map map; // The map that is being edited

	std::function<void()> onStateChange;
	void notifyStateChange();

public: // Functions
	// Map handling

	// Adds an action to the action queue (this allows the user to undo the action)
	// Invalidates the action pointer
	void addBatch(std::unique_ptr<BatchAction> action, int stacking_delay = 0);
	void addAction(std::unique_ptr<Action> action, int stacking_delay = 0);

	// Selection
	bool hasSelection() const {
		return selection.size() != 0;
	}
	// Some simple actions that work on the map (these will work through the undo queue)
	// Moves the selected area by the offset
	void moveSelection(Position offset);
	// Deletes all selected items
	void destroySelection();
	// Borderizes the selected region
	void borderizeSelection();
	// Randomizes the ground in the selected region
	void randomizeSelection();

	// Same as above although it applies to the entire map
	// action queue is flushed when these functions are called
	// showdialog is whether a progress bar should be shown
	void borderizeMap(bool showdialog);
	void randomizeMap(bool showdialog);
	void clearInvalidHouseTiles(bool showdialog);
	void clearModifiedTileState(bool showdialog);

	// Draw using the current brush to the target position
	// alt is whether the ALT key is pressed
	void draw(const Position& offset, bool alt);
	void undraw(const Position& offset, bool alt);
	void draw(const PositionVector& posvec, bool alt);
	void draw(const PositionVector& todraw, PositionVector& toborder, bool alt);
	void undraw(const PositionVector& posvec, bool alt);
	void undraw(const PositionVector& todraw, PositionVector& toborder, bool alt);

protected:
	void drawInternal(const Position offset, bool alt, bool dodraw);
	void drawInternal(const PositionVector& posvec, bool alt, bool dodraw);
	void drawInternal(const PositionVector& todraw, PositionVector& toborder, bool alt, bool dodraw);

	Editor(const Editor&);
	Editor& operator=(const Editor&);
};

inline void Editor::draw(const Position& offset, bool alt) {
	drawInternal(offset, alt, true);
}
inline void Editor::undraw(const Position& offset, bool alt) {
	drawInternal(offset, alt, false);
}
inline void Editor::draw(const PositionVector& posvec, bool alt) {
	drawInternal(posvec, alt, true);
}
inline void Editor::draw(const PositionVector& todraw, PositionVector& toborder, bool alt) {
	drawInternal(todraw, toborder, alt, true);
}
inline void Editor::undraw(const PositionVector& posvec, bool alt) {
	drawInternal(posvec, alt, false);
}
inline void Editor::undraw(const PositionVector& todraw, PositionVector& toborder, bool alt) {
	drawInternal(todraw, toborder, alt, false);
}

#endif
