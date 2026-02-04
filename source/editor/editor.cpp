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

#include "app/main.h"

#include "editor/editor.h"
#include "editor/action_queue.h"
#include "game/materials.h"
#include "map/map.h"
#include "game/complexitem.h"
#include "app/settings.h"
#include "ui/gui.h"
#include "rendering/ui/map_display.h"
#include "brushes/brush.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/wall/wall_brush.h"
#include "brushes/table/table_brush.h"
#include "brushes/carpet/carpet_brush.h"
#include "brushes/waypoint/waypoint_brush.h"
#include "brushes/house/house_exit_brush.h"
#include "brushes/doodad/doodad_brush.h"
#include "brushes/creature/creature_brush.h"
#include "brushes/spawn/spawn_brush.h"

#include "live/live_server.h"
#include "live/live_client.h"
#include "live/live_action.h"

#include "editor/operations/draw_operations.h"

#include <spdlog/spdlog.h>

Editor::Editor(CopyBuffer& copybuffer, const MapVersion& version) :
	live_manager(*this),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr) {
	spdlog::info("Editor created (Empty) [Editor={}]", (void*)this);
	map.convert(version);
	map.initializeEmpty();
}

Editor::Editor(CopyBuffer& copybuffer, const MapVersion& version, const FileName& fn) :
	live_manager(*this),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr) {
	spdlog::info("Editor created (From File) [Editor={}]", (void*)this);
	// EditorPersistence handles version checking internally or assumes compatibility
	// Usage of "version" parameter here might be redundant for loading but good for consistency/future use
	EditorPersistence::loadMap(*this, fn);
}

Editor::Editor(CopyBuffer& copybuffer, const MapVersion& version, LiveClient* client) :
	live_manager(*this, std::unique_ptr<LiveClient>(client)),
	actionQueue(newd NetworkedActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr) {
	spdlog::info("Editor created (Live Client) [Editor={}]", (void*)this);
	map.convert(version);
}

Editor::~Editor() {
	spdlog::info("Editor destroying [Editor={}]", (void*)this);
	if (live_manager.IsLive()) {
		live_manager.CloseServer();
	}

	UnnamedRenderingLock();
	selection.clear();
	delete actionQueue;
	spdlog::info("Editor destroyed [Editor={}]", (void*)this);
}

void Editor::notifyStateChange() {
	if (onStateChange) {
		onStateChange();
	}
}

void Editor::addBatch(std::unique_ptr<BatchAction> action, int stacking_delay) {
	actionQueue->addBatch(std::move(action), stacking_delay);
	notifyStateChange();
}

void Editor::addAction(std::unique_ptr<Action> action, int stacking_delay) {
	actionQueue->addAction(std::move(action), stacking_delay);
	notifyStateChange();
}

void Editor::borderizeSelection() {
	SelectionOperations::borderizeSelection(*this);
}

void Editor::borderizeMap(bool showdialog) {
	MapProcessor::borderizeMap(*this, showdialog);
}

void Editor::randomizeSelection() {
	SelectionOperations::randomizeSelection(*this);
}

void Editor::randomizeMap(bool showdialog) {
	MapProcessor::randomizeMap(*this, showdialog);
}

void Editor::clearInvalidHouseTiles(bool showdialog) {
	MapProcessor::clearInvalidHouseTiles(*this, showdialog);
}

void Editor::clearModifiedTileState(bool showdialog) {
	MapProcessor::clearModifiedTileState(*this, showdialog);
}

void Editor::moveSelection(Position offset) {
	SelectionOperations::moveSelection(*this, offset);
}

void Editor::destroySelection() {
	SelectionOperations::destroySelection(*this);
}

// Helper functions moved to SelectionOperations

void Editor::drawInternal(Position offset, bool alt, bool dodraw) {
	DrawOperations::draw(*this, offset, alt, dodraw);
}

void Editor::drawInternal(const PositionVector& tilestodraw, bool alt, bool dodraw) {
	DrawOperations::draw(*this, tilestodraw, alt, dodraw);
}

void Editor::drawInternal(const PositionVector& tilestodraw, PositionVector& tilestoborder, bool alt, bool dodraw) {
	DrawOperations::draw(*this, tilestodraw, tilestoborder, alt, dodraw);
}

///////////////////////////////////////////////////////////////////////////////
// Live!
