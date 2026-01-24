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
#include "game/materials.h"
#include "map/map.h"
#include "game/complexitem.h"
#include "app/settings.h"
#include "ui/gui.h"
#include "rendering/ui/map_display.h"
#include "brushes/brush.h"
#include "brushes/ground_brush.h"
#include "brushes/wall_brush.h"
#include "brushes/table_brush.h"
#include "brushes/carpet_brush.h"
#include "brushes/waypoint_brush.h"
#include "brushes/house_exit_brush.h"
#include "brushes/doodad_brush.h"
#include "brushes/creature_brush.h"
#include "brushes/spawn_brush.h"

#include "live/live_server.h"
#include "live/live_client.h"
#include "live/live_action.h"

#include "editor/operations/draw_operations.h"

Editor::Editor(CopyBuffer& copybuffer) :
	live_manager(*this),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr) {
	wxString error;
	wxArrayString warnings;
	bool ok = true;

	ClientVersionID defaultVersion = ClientVersionID(g_settings.getInteger(Config::DEFAULT_CLIENT_VERSION));
	if (defaultVersion == CLIENT_VERSION_NONE) {
		defaultVersion = ClientVersion::getLatestVersion()->getID();
	}

	if (g_gui.GetCurrentVersionID() != defaultVersion) {
		if (g_gui.CloseAllEditors()) {
			ok = g_gui.LoadVersion(defaultVersion, error, warnings);
			g_gui.PopupDialog("Error", error, wxOK);
			g_gui.ListDialog("Warnings", warnings);
		} else {
			throw std::runtime_error("All maps of different versions were not closed.");
		}
	}

	if (!ok) {
		throw std::runtime_error("Couldn't load client version");
	}

	MapVersion version;
	version.otbm = g_gui.GetCurrentVersion().getPrefferedMapVersionID();
	version.client = g_gui.GetCurrentVersionID();
	map.convert(version);

	map.height = 2048;
	map.width = 2048;

	static int unnamed_counter = 0;

	std::string sname = "Untitled-" + i2s(++unnamed_counter);
	map.name = sname + ".otbm";
	map.spawnfile = sname + "-spawn.xml";
	map.housefile = sname + "-house.xml";
	map.waypointfile = sname + "-waypoint.xml";
	map.description = "No map description available.";
	map.unnamed = true;

	map.doChange();
}

Editor::Editor(CopyBuffer& copybuffer, const FileName& fn) :
	live_manager(*this),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr) {
	EditorPersistence::loadMap(*this, fn);
}

Editor::Editor(CopyBuffer& copybuffer, LiveClient* client) :
	live_manager(*this, client),
	actionQueue(newd NetworkedActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr) {
	;
}

Editor::~Editor() {
	if (IsLive()) {
		CloseLiveServer();
	}

	UnnamedRenderingLock();
	selection.clear();
	delete actionQueue;
}

void Editor::addBatch(BatchAction* action, int stacking_delay) {
	actionQueue->addBatch(action, stacking_delay);
	g_gui.UpdateMenus();
}

void Editor::addAction(Action* action, int stacking_delay) {
	actionQueue->addAction(action, stacking_delay);
	g_gui.UpdateMenus();
}

void Editor::saveMap(FileName filename, bool showdialog) {
	EditorPersistence::saveMap(*this, filename, showdialog);
}

bool Editor::importMiniMap(FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset) {
	return false;
}

bool Editor::exportMiniMap(FileName filename, int floor /*= GROUND_LAYER*/, bool displaydialog) {
	return map.exportMinimap(filename, floor, displaydialog);
}

bool Editor::exportSelectionAsMiniMap(FileName directory, wxString fileName) {
	return EditorPersistence::exportSelectionAsMiniMap(*this, directory, fileName);
}

bool Editor::importMap(FileName filename, int import_x_offset, int import_y_offset, ImportType house_import_type, ImportType spawn_import_type) {
	return EditorPersistence::importMap(*this, filename, import_x_offset, import_y_offset, house_import_type, spawn_import_type);
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

bool Editor::IsLiveClient() const {
	return live_manager.IsClient();
}

bool Editor::IsLiveServer() const {
	return live_manager.IsServer();
}

bool Editor::IsLive() const {
	return live_manager.IsLive();
}

bool Editor::IsLocal() const {
	return live_manager.IsLocal();
}

LiveClient* Editor::GetLiveClient() const {
	return live_manager.GetClient();
}

LiveServer* Editor::GetLiveServer() const {
	return live_manager.GetServer();
}

LiveSocket& Editor::GetLive() const {
	return live_manager.GetSocket();
}

LiveServer* Editor::StartLiveServer() {
	return live_manager.StartServer();
}

void Editor::BroadcastNodes(DirtyList& dirtyList) {
	live_manager.BroadcastNodes(dirtyList);
}

void Editor::CloseLiveServer() {
	live_manager.CloseServer();
}

void Editor::QueryNode(int ndx, int ndy, bool underground) {
	ASSERT(live_manager.GetClient());
	live_manager.GetClient()->queryNode(ndx, ndy, underground);
}

void Editor::SendNodeRequests() {
	if (live_manager.GetClient()) {
		live_manager.GetClient()->sendNodeRequests();
	}
}
