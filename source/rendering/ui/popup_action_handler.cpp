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
#include "rendering/ui/popup_action_handler.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "map/tile.h"
#include "game/item.h"
#include "ui/gui.h"
#include "ui/browse_tile_window.h"
#include "ui/tileset_window.h"
#include "ui/dialog_helper.h"
#include "brushes/brush.h"
#include "brushes/door/door_brush.h"

void PopupActionHandler::RotateItem(Editor& editor) {
	Tile* tile = editor.selection.getSelectedTile();

	std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_ROTATE_ITEM);

	std::unique_ptr<Tile> new_tile(tile->deepCopy(editor.map));

	ItemVector selected_items = new_tile->getSelectedItems();
	ASSERT(!selected_items.empty());

	selected_items.front()->doRotate();

	action->addChange(std::make_unique<Change>(new_tile.release()));

	editor.actionQueue->addAction(std::move(action));
	g_gui.RefreshView();
}

void PopupActionHandler::GotoDestination(Editor& editor) {
	Tile* tile = editor.selection.getSelectedTile();
	ItemVector selected_items = tile->getSelectedItems();
	ASSERT(!selected_items.empty());
	Teleport* teleport = dynamic_cast<Teleport*>(selected_items.front());
	if (teleport) {
		Position pos = teleport->getDestination();
		g_gui.SetScreenCenterPosition(pos);
	}
}

void PopupActionHandler::SwitchDoor(Editor& editor) {
	Tile* tile = editor.selection.getSelectedTile();

	std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_SWITCHDOOR);

	std::unique_ptr<Tile> new_tile(tile->deepCopy(editor.map));

	ItemVector selected_items = new_tile->getSelectedItems();
	ASSERT(!selected_items.empty());

	DoorBrush::switchDoor(selected_items.front());

	action->addChange(std::make_unique<Change>(new_tile.release()));

	editor.actionQueue->addAction(std::move(action));
	g_gui.RefreshView();
}

void PopupActionHandler::BrowseTile(Editor& editor, int cursor_x, int cursor_y) {
	if (editor.selection.size() != 1) {
		return;
	}

	Tile* tile = editor.selection.getSelectedTile();
	if (!tile) {
		return;
	}
	ASSERT(tile->isSelected());
	std::unique_ptr<Tile> new_tile(tile->deepCopy(editor.map));

	wxDialog* w = new BrowseTileWindow(g_gui.root, new_tile.get(), wxPoint(cursor_x, cursor_y));

	int ret = w->ShowModal();
	if (ret != 0) {
		std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_DELETE_TILES);
		action->addChange(std::make_unique<Change>(new_tile.release()));
		editor.addAction(std::move(action));
	}

	w->Destroy();
}

void PopupActionHandler::OpenProperties(Editor& editor) {
	if (editor.selection.size() != 1) {
		return;
	}

	Tile* tile = editor.selection.getSelectedTile();
	if (tile) {
		DialogHelper::OpenProperties(editor, tile);
	}
}

void PopupActionHandler::SelectMoveTo(Editor& editor) {
	if (editor.selection.size() != 1) {
		return;
	}

	Tile* tile = editor.selection.getSelectedTile();
	if (!tile) {
		return;
	}
	ASSERT(tile->isSelected());
	std::unique_ptr<Tile> new_tile(tile->deepCopy(editor.map));

	wxDialog* w = nullptr;

	ItemVector selected_items = new_tile->getSelectedItems();

	Item* item = nullptr;
	int count = 0;
	for (ItemVector::iterator it = selected_items.begin(); it != selected_items.end(); ++it) {
		++count;
		if ((*it)->isSelected()) {
			item = *it;
		}
	}

	if (item) {
		w = newd TilesetWindow(g_gui.root, &editor.map, new_tile.get(), item);
	} else {
		return;
	}

	int ret = w->ShowModal();
	if (ret != 0) {
		std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_CHANGE_PROPERTIES);
		action->addChange(std::make_unique<Change>(new_tile.release()));
		editor.addAction(std::move(action));

		g_gui.RebuildPalettes();
	}
	w->Destroy();
}
