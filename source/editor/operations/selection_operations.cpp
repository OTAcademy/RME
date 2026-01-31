//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "editor/operations/selection_operations.h"
#include "editor/editor.h"
#include "editor/action.h"
#include "editor/action_queue.h"
#include "editor/selection.h"
#include "map/map.h"
#include "brushes/ground/ground_brush.h"
#include "app/settings.h"
#include "ui/gui.h"

#include "brushes/doodad/doodad_brush.h"

void SelectionOperations::doSurroundingBorders(DoodadBrush* doodad_brush, PositionList& tilestoborder, Tile* buffer_tile, Tile* new_tile) {
	if (doodad_brush->doNewBorders() && g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		tilestoborder.push_back(Position(new_tile->getPosition().x, new_tile->getPosition().y, new_tile->getPosition().z));
		if (buffer_tile->hasGround()) {
			for (int y = -1; y <= 1; y++) {
				for (int x = -1; x <= 1; x++) {
					tilestoborder.push_back(Position(new_tile->getPosition().x + x, new_tile->getPosition().y + y, new_tile->getPosition().z));
				}
			}
		} else if (buffer_tile->hasWall()) {
			tilestoborder.push_back(Position(new_tile->getPosition().x, new_tile->getPosition().y - 1, new_tile->getPosition().z));
			tilestoborder.push_back(Position(new_tile->getPosition().x - 1, new_tile->getPosition().y, new_tile->getPosition().z));
			tilestoborder.push_back(Position(new_tile->getPosition().x + 1, new_tile->getPosition().y, new_tile->getPosition().z));
			tilestoborder.push_back(Position(new_tile->getPosition().x, new_tile->getPosition().y + 1, new_tile->getPosition().z));
		}
	}
}

void SelectionOperations::removeDuplicateWalls(Tile* buffer, Tile* tile) {
	for (ItemVector::const_iterator iter = buffer->items.begin(); iter != buffer->items.end(); ++iter) {
		if ((*iter)->getWallBrush()) {
			tile->cleanWalls((*iter)->getWallBrush());
		}
	}
}

void SelectionOperations::borderizeSelection(Editor& editor) {
	if (editor.selection.size() == 0) {
		g_gui.SetStatusText("No items selected. Can't borderize.");
	}

	std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_BORDERIZE);
	for (Tile* tile : editor.selection) {
		Tile* newTile = tile->deepCopy(editor.map);
		newTile->borderize(&editor.map);
		newTile->select();
		action->addChange(std::make_unique<Change>(newTile));
	}
	editor.addAction(std::move(action));
}

void SelectionOperations::randomizeSelection(Editor& editor) {
	if (editor.selection.size() == 0) {
		g_gui.SetStatusText("No items selected. Can't randomize.");
	}

	std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_RANDOMIZE);
	for (Tile* tile : editor.selection) {
		Tile* newTile = tile->deepCopy(editor.map);
		GroundBrush* groundBrush = newTile->getGroundBrush();
		if (groundBrush && groundBrush->isReRandomizable()) {
			groundBrush->draw(&editor.map, newTile, nullptr);

			Item* oldGround = tile->ground;
			Item* newGround = newTile->ground;
			if (oldGround && newGround) {
				newGround->setActionID(oldGround->getActionID());
				newGround->setUniqueID(oldGround->getUniqueID());
			}

			newTile->select();
			action->addChange(std::make_unique<Change>(newTile));
		}
	}
	editor.addAction(std::move(action));
}

void SelectionOperations::moveSelection(Editor& editor, Position offset) {
	std::unique_ptr<BatchAction> batchAction = editor.actionQueue->createBatch(ACTION_MOVE); // Our saved action batch, for undo!
	std::unique_ptr<Action> action;

	// Remove tiles from the map
	action = editor.actionQueue->createAction(batchAction.get()); // Our action!
	bool doborders = false;
	TileSet tmp_storage;

	// Update the tiles with the newd positions
	for (auto it = editor.selection.begin(); it != editor.selection.end(); ++it) {
		// First we get the old tile and it's position
		Tile* tile = (*it);
		// const Position pos = tile->getPosition();

		// Create the duplicate source tile, which will replace the old one later
		Tile* old_src_tile = tile;
		Tile* new_src_tile;

		new_src_tile = old_src_tile->deepCopy(editor.map);

		Tile* tmp_storage_tile = editor.map.allocator(tile->getLocation());

		// Get all the selected items from the NEW source tile and iterate through them
		// This transfers ownership to the temporary tile
		ItemVector tile_selection = new_src_tile->popSelectedItems();
		for (ItemVector::iterator iit = tile_selection.begin(); iit != tile_selection.end(); iit++) {
			// Add the copied item to the newd destination tile,
			Item* item = (*iit);
			tmp_storage_tile->addItem(item);
		}
		// Move spawns
		if (new_src_tile->spawn && new_src_tile->spawn->isSelected()) {
			tmp_storage_tile->spawn = new_src_tile->spawn;
			new_src_tile->spawn = nullptr;
		}
		// Move creatures
		if (new_src_tile->creature && new_src_tile->creature->isSelected()) {
			tmp_storage_tile->creature = new_src_tile->creature;
			new_src_tile->creature = nullptr;
		}

		// Move house data & tile status if ground is transferred
		if (tmp_storage_tile->ground) {
			tmp_storage_tile->house_id = new_src_tile->house_id;
			new_src_tile->house_id = 0;
			tmp_storage_tile->setMapFlags(new_src_tile->getMapFlags());
			new_src_tile->setMapFlags(TILESTATE_NONE);
			doborders = true;
		}

		tmp_storage.push_back(tmp_storage_tile);
		// Add the tile copy to the action
		action->addChange(std::make_unique<Change>(new_src_tile));
	}
	// Commit changes to map
	batchAction->addAndCommitAction(std::move(action));

	// Remove old borders (and create some newd?)
	if (g_settings.getInteger(Config::USE_AUTOMAGIC) && g_settings.getInteger(Config::BORDERIZE_DRAG) && editor.selection.size() < size_t(g_settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD))) {
		action = editor.actionQueue->createAction(batchAction.get());
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for (TileSet::iterator it = tmp_storage.begin(); it != tmp_storage.end(); ++it) {
			Position pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = editor.map.getTile(pos.x, pos.y, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x - 1, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x + 1, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x - 1, pos.y, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x + 1, pos.y, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x - 1, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
			t = editor.map.getTile(pos.x + 1, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
			}
		}
		// Remove duplicates
		borderize_tiles.sort();
		borderize_tiles.unique();
		// Do le borders!
		for (TileList::iterator it = borderize_tiles.begin(); it != borderize_tiles.end(); ++it) {
			Tile* tile = *it;
			Tile* new_tile = (*it)->deepCopy(editor.map);
			if (doborders) {
				new_tile->borderize(&editor.map);
			}
			new_tile->wallize(&editor.map);
			new_tile->tableize(&editor.map);
			new_tile->carpetize(&editor.map);
			if (tile->ground && tile->ground->isSelected()) {
				new_tile->selectGround();
			}
			action->addChange(std::make_unique<Change>(new_tile));
		}
		// Commit changes to map
		batchAction->addAndCommitAction(std::move(action));
	}

	// New action for adding the destination tiles
	action = editor.actionQueue->createAction(batchAction.get());
	for (TileSet::iterator it = tmp_storage.begin(); it != tmp_storage.end(); ++it) {
		Tile* tile = (*it);
		const Position old_pos = tile->getPosition();
		Position new_pos;

		new_pos = old_pos - offset;

		if (new_pos.z < 0 && new_pos.z > MAP_MAX_LAYER) {
			delete tile;
			continue;
		}
		// Create the duplicate dest tile, which will replace the old one later
		TileLocation* location = editor.map.createTileL(new_pos);
		Tile* old_dest_tile = location->get();
		Tile* new_dest_tile = nullptr;

		if (g_settings.getInteger(Config::MERGE_MOVE) || !tile->ground) {
			// Move items
			if (old_dest_tile) {
				new_dest_tile = old_dest_tile->deepCopy(editor.map);
			} else {
				new_dest_tile = editor.map.allocator(location);
			}
			new_dest_tile->merge(tile);
			delete tile;
		} else {
			// Replace tile instead of just merge
			tile->setLocation(location);
			new_dest_tile = tile;
		}

		action->addChange(std::make_unique<Change>(new_dest_tile));
	}

	// Commit changes to the map
	batchAction->addAndCommitAction(std::move(action));

	// Create borders
	if (g_settings.getInteger(Config::USE_AUTOMAGIC) && g_settings.getInteger(Config::BORDERIZE_DRAG) && editor.selection.size() < size_t(g_settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD))) {
		action = editor.actionQueue->createAction(batchAction.get());
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for (auto it = editor.selection.begin(); it != editor.selection.end(); it++) {
			bool add_me = false; // If this tile is touched
			Position pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = editor.map.getTile(pos.x - 1, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x - 1, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x + 1, pos.y - 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x - 1, pos.y, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x + 1, pos.y, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x - 1, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			t = editor.map.getTile(pos.x + 1, pos.y + 1, pos.z);
			if (t && !t->isSelected()) {
				borderize_tiles.push_back(t);
				add_me = true;
			}
			if (add_me) {
				borderize_tiles.push_back(*it);
			}
		}
		// Remove duplicates
		borderize_tiles.sort();
		borderize_tiles.unique();
		// Do le borders!
		for (TileList::iterator it = borderize_tiles.begin(); it != borderize_tiles.end(); it++) {
			Tile* tile = *it;
			if (tile->ground) {
				if (tile->ground->getGroundBrush()) {
					Tile* new_tile = tile->deepCopy(editor.map);

					if (doborders) {
						new_tile->borderize(&editor.map);
					}

					new_tile->wallize(&editor.map);
					new_tile->tableize(&editor.map);
					new_tile->carpetize(&editor.map);
					if (tile->ground->isSelected()) {
						new_tile->selectGround();
					}

					action->addChange(std::make_unique<Change>(new_tile));
				}
			}
		}
		// Commit changes to map
		batchAction->addAndCommitAction(std::move(action));
	}

	// Store the action for undo
	editor.addBatch(std::move(batchAction));
	editor.selection.updateSelectionCount();
}

void SelectionOperations::destroySelection(Editor& editor) {
	if (editor.selection.size() == 0) {
		g_gui.SetStatusText("No selected items to delete.");
	} else {
		int tile_count = 0;
		int item_count = 0;
		PositionList tilestoborder;

		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DELETE_TILES);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());

		for (auto it = editor.selection.begin(); it != editor.selection.end(); ++it) {
			tile_count++;

			Tile* tile = *it;
			Tile* newtile = tile->deepCopy(editor.map);

			ItemVector tile_selection = newtile->popSelectedItems();
			for (ItemVector::iterator iit = tile_selection.begin(); iit != tile_selection.end(); ++iit) {
				++item_count;
				// Delete the items from the tile
				delete *iit;
			}

			if (newtile->creature && newtile->creature->isSelected()) {
				delete newtile->creature;
				newtile->creature = nullptr;
			}

			if (newtile->spawn && newtile->spawn->isSelected()) {
				delete newtile->spawn;
				newtile->spawn = nullptr;
			}

			if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				for (int y = -1; y <= 1; y++) {
					for (int x = -1; x <= 1; x++) {
						tilestoborder.push_back(
							Position(tile->getPosition().x + x, tile->getPosition().y + y, tile->getPosition().z)
						);
					}
				}
			}
			action->addChange(std::make_unique<Change>(newtile));
		}

		batch->addAndCommitAction(std::move(action));

		if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Remove duplicates
			tilestoborder.sort();
			tilestoborder.unique();

			action = editor.actionQueue->createAction(batch.get());
			for (PositionList::iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = editor.map.createTileL(*it);
				Tile* tile = location->get();

				if (tile) {
					Tile* new_tile = tile->deepCopy(editor.map);
					new_tile->borderize(&editor.map);
					new_tile->wallize(&editor.map);
					new_tile->tableize(&editor.map);
					new_tile->carpetize(&editor.map);
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
		wxString ss;
		ss << "Deleted " << tile_count << " tile" << (tile_count > 1 ? "s" : "") << " (" << item_count << " item" << (item_count > 1 ? "s" : "") << ")";
		g_gui.SetStatusText(ss);
	}
}
