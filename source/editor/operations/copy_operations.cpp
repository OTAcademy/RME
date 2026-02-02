//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "editor/operations/copy_operations.h"
#include "editor/copybuffer.h"
#include "editor/editor.h"
#include "ui/gui.h"
#include "game/creature.h"
#include "game/spawn.h"
#include "map/map.h"
#include "editor/action.h"
#include "editor/action_queue.h"
#include "app/settings.h"

#include <sstream>

void CopyOperations::copy(Editor& editor, CopyBuffer& buffer, int floor) {
	if (editor.selection.empty()) {
		g_gui.SetStatusText("No tiles to copy.");
		return;
	}

	buffer.clear();
	buffer.tiles = std::make_unique<BaseMap>();

	int tile_count = 0;
	int item_count = 0;
	buffer.copyPos = Position(0xFFFF, 0xFFFF, floor);

	for (Tile* tile : editor.selection) {
		++tile_count;

		TileLocation* newlocation = buffer.tiles->createTileL(tile->getPosition());
		Tile* copied_tile = buffer.tiles->allocator(newlocation);

		if (tile->ground && tile->ground->isSelected()) {
			copied_tile->house_id = tile->house_id;
			copied_tile->setMapFlags(tile->getMapFlags());
		}

		ItemVector tile_selection = tile->getSelectedItems();

		for (Item* item : tile_selection) {
			++item_count;
			// Copy items to copybuffer
			copied_tile->addItem(item->deepCopy());
		}

		if (tile->creature && tile->creature->isSelected()) {
			copied_tile->creature = tile->creature->deepCopy();
		}
		if (tile->spawn && tile->spawn->isSelected()) {
			copied_tile->spawn = tile->spawn->deepCopy();
		}

		buffer.tiles->setTile(copied_tile);

		if (copied_tile->getX() < buffer.copyPos.x) {
			buffer.copyPos.x = copied_tile->getX();
		}

		if (copied_tile->getY() < buffer.copyPos.y) {
			buffer.copyPos.y = copied_tile->getY();
		}
	}

	std::ostringstream ss;
	ss << "Copied " << tile_count << " tile" << (tile_count > 1 ? "s" : "") << " (" << item_count << " item" << (item_count > 1 ? "s" : "") << ")";
	g_gui.SetStatusText(wxstr(ss.str()));
}

void CopyOperations::cut(Editor& editor, CopyBuffer& buffer, int floor) {
	if (editor.selection.empty()) {
		g_gui.SetStatusText("No tiles to cut.");
		return;
	}

	buffer.clear();
	buffer.tiles = std::make_unique<BaseMap>();

	int tile_count = 0;
	int item_count = 0;
	buffer.copyPos = Position(0xFFFF, 0xFFFF, floor);

	std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_CUT_TILES);
	std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());

	PositionList tilestoborder;

	for (Tile* tile : editor.selection) {
		tile_count++;

		Tile* newtile = tile->deepCopy(editor.map);
		Tile* copied_tile = buffer.tiles->allocator(tile->getLocation());

		if (tile->ground && tile->ground->isSelected()) {
			copied_tile->house_id = newtile->house_id;
			newtile->house_id = 0;
			copied_tile->setMapFlags(tile->getMapFlags());
			newtile->setMapFlags(TILESTATE_NONE);
		}

		ItemVector tile_selection = newtile->popSelectedItems();

		for (Item* item : tile_selection) {
			item_count++;
			// Add items to copybuffer
			copied_tile->addItem(item);
		}

		if (newtile->creature && newtile->creature->isSelected()) {
			copied_tile->creature = newtile->creature;
			newtile->creature = nullptr;
		}

		if (newtile->spawn && newtile->spawn->isSelected()) {
			copied_tile->spawn = newtile->spawn;
			newtile->spawn = nullptr;
		}

		buffer.tiles->setTile(copied_tile->getPosition(), copied_tile);

		if (copied_tile->getX() < buffer.copyPos.x) {
			buffer.copyPos.x = copied_tile->getX();
		}

		if (copied_tile->getY() < buffer.copyPos.y) {
			buffer.copyPos.y = copied_tile->getY();
		}

		if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			for (int y = -1; y <= 1; y++) {
				for (int x = -1; x <= 1; x++) {
					tilestoborder.push_back(Position(tile->getX() + x, tile->getY() + y, tile->getZ()));
				}
			}
		}
		action->addChange(std::make_unique<Change>(newtile));
	}

	batch->addAndCommitAction(std::move(action));

	// Remove duplicates
	tilestoborder.sort();
	tilestoborder.unique();

	if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		action = editor.actionQueue->createAction(batch.get());
		for (const Position& pos : tilestoborder) {
			TileLocation* location = editor.map.createTileL(pos);
			if (location->get()) {
				Tile* new_tile = location->get()->deepCopy(editor.map);
				new_tile->borderize(&editor.map);
				new_tile->wallize(&editor.map);
				action->addChange(std::make_unique<Change>(new_tile));
			} else {
				Tile* new_tile = editor.map.allocator(location);
				new_tile->borderize(&editor.map);
				if (new_tile->size()) {
					action->addChange(std::make_unique<Change>(new_tile));
				} else {
					delete new_tile;
				}
			}
		}

		batch->addAndCommitAction(std::move(action));
	}

	editor.addBatch(std::move(batch));
	std::stringstream ss;
	ss << "Cut out " << tile_count << " tile" << (tile_count > 1 ? "s" : "") << " (" << item_count << " item" << (item_count > 1 ? "s" : "") << ")";
	g_gui.SetStatusText(wxstr(ss.str()));
}

void CopyOperations::paste(Editor& editor, CopyBuffer& buffer, const Position& toPosition) {
	if (!buffer.tiles) {
		return;
	}

	std::unique_ptr<BatchAction> batchAction = editor.actionQueue->createBatch(ACTION_PASTE_TILES);
	std::unique_ptr<Action> action = editor.actionQueue->createAction(batchAction.get());
	for (TileLocation* location : *buffer.tiles) {
		Tile* buffer_tile = location->get();
		Position pos = buffer_tile->getPosition() - buffer.copyPos + toPosition;

		if (!pos.isValid()) {
			continue;
		}

		TileLocation* dest_location = editor.map.createTileL(pos);
		Tile* copy_tile = buffer_tile->deepCopy(editor.map);
		Tile* old_dest_tile = dest_location->get();
		Tile* new_dest_tile = nullptr;
		copy_tile->setLocation(dest_location);

		if (g_settings.getInteger(Config::MERGE_PASTE) || !copy_tile->ground) {
			if (old_dest_tile) {
				new_dest_tile = old_dest_tile->deepCopy(editor.map);
			} else {
				new_dest_tile = editor.map.allocator(dest_location);
			}
			new_dest_tile->merge(copy_tile);
			delete copy_tile;
		} else {
			// If the copied tile has ground, replace target tile
			new_dest_tile = copy_tile;
		}

		// Add all surrounding tiles to the map, so they get borders
		editor.map.createTile(pos.x - 1, pos.y - 1, pos.z);
		editor.map.createTile(pos.x, pos.y - 1, pos.z);
		editor.map.createTile(pos.x + 1, pos.y - 1, pos.z);
		editor.map.createTile(pos.x - 1, pos.y, pos.z);
		editor.map.createTile(pos.x + 1, pos.y, pos.z);
		editor.map.createTile(pos.x - 1, pos.y + 1, pos.z);
		editor.map.createTile(pos.x, pos.y + 1, pos.z);
		editor.map.createTile(pos.x + 1, pos.y + 1, pos.z);

		action->addChange(std::make_unique<Change>(new_dest_tile));
	}
	batchAction->addAndCommitAction(std::move(action));

	if (g_settings.getInteger(Config::USE_AUTOMAGIC) && g_settings.getInteger(Config::BORDERIZE_PASTE)) {
		action = editor.actionQueue->createAction(batchAction.get());
		TileList borderize_tiles;
		Map& map = editor.map;

		// Go through all modified (selected) tiles (might be slow)
		for (TileLocation* location : *buffer.tiles) {
			bool add_me = false; // If this tile is touched
			Position pos = location->getPosition() - buffer.copyPos + toPosition;
			if (pos.z < 0 || pos.z >= MAP_LAYERS) {
				continue;
			}
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x - 1, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = map.getTile(pos.x, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = map.getTile(pos.x + 1, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = map.getTile(pos.x - 1, pos.y, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = map.getTile(pos.x + 1, pos.y, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = map.getTile(pos.x - 1, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = map.getTile(pos.x, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = map.getTile(pos.x + 1, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			if (add_me) {
				borderize_tiles.push_back(map.getTile(pos));
			}
		}
		// Remove duplicates
		borderize_tiles.sort();
		borderize_tiles.unique();

		for (Tile* tile : borderize_tiles) {
			if (tile) {
				Tile* newTile = tile->deepCopy(editor.map);
				newTile->borderize(&map);

				if (tile->ground && tile->ground->isSelected()) {
					newTile->selectGround();
				}

				newTile->wallize(&map);
				action->addChange(std::make_unique<Change>(newTile));
			}
		}

		// Commit changes to map
		batchAction->addAndCommitAction(std::move(action));
	}

	editor.addBatch(std::move(batchAction));
}
