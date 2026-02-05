//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "map/operations/map_processor.h"
#include "editor/editor.h"
#include "map/map.h"
#include "ui/gui.h"
#include "brushes/ground/ground_brush.h"

void MapProcessor::borderizeMap(Editor& editor, bool showdialog) {
	if (showdialog) {
		g_gui.CreateLoadBar("Borderizing map...");
	}

	uint64_t tiles_done = 0;
	for (TileLocation& tileLocation : editor.map) {
		if (showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(static_cast<int32_t>(tiles_done / double(editor.map.getTileCount()) * 100.0));
		}

		Tile* tile = tileLocation.get();
		ASSERT(tile);

		tile->borderize(&editor.map);
		++tiles_done;
	}

	if (showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void MapProcessor::randomizeMap(Editor& editor, bool showdialog) {
	if (showdialog) {
		g_gui.CreateLoadBar("Randomizing map...");
	}

	uint64_t tiles_done = 0;
	for (TileLocation& tileLocation : editor.map) {
		if (showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(static_cast<int32_t>(tiles_done / double(editor.map.getTileCount()) * 100.0));
		}

		Tile* tile = tileLocation.get();
		ASSERT(tile);

		GroundBrush* groundBrush = tile->getGroundBrush();
		if (groundBrush) {
			Item* oldGround = tile->ground;

			uint16_t actionId, uniqueId;
			if (oldGround) {
				actionId = oldGround->getActionID();
				uniqueId = oldGround->getUniqueID();
			} else {
				actionId = 0;
				uniqueId = 0;
			}
			groundBrush->draw(&editor.map, tile, nullptr);

			Item* newGround = tile->ground;
			if (newGround) {
				newGround->setActionID(actionId);
				newGround->setUniqueID(uniqueId);
			}
			tile->update();
		}
		++tiles_done;
	}

	if (showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void MapProcessor::clearInvalidHouseTiles(Editor& editor, bool showdialog) {
	if (showdialog) {
		g_gui.CreateLoadBar("Clearing invalid house tiles...");
	}

	Houses& houses = editor.map.houses;

	HouseMap::iterator iter = houses.begin();
	while (iter != houses.end()) {
		House* h = iter->second.get();
		if (editor.map.towns.getTown(h->townid) == nullptr) {
#ifdef __VISUALC__ // C++0x compliance to some degree :)
			iter = houses.erase(iter);
#else // Bulky, slow way
			HouseMap::iterator tmp_iter = iter;
			++tmp_iter;
			uint32_t next_key = 0;
			if (tmp_iter != houses.end()) {
				next_key = tmp_iter->first;
			}
			houses.erase(iter);
			if (next_key != 0) {
				iter = houses.find(next_key);
			} else {
				iter = houses.end();
			}
#endif
		} else {
			++iter;
		}
	}

	uint64_t tiles_done = 0;
	for (MapIterator map_iter = editor.map.begin(); map_iter != editor.map.end(); ++map_iter) {
		if (showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(int(tiles_done / double(editor.map.getTileCount()) * 100.0));
		}

		Tile* tile = map_iter->get();
		ASSERT(tile);
		if (tile->isHouseTile()) {
			if (houses.getHouse(tile->getHouseID()) == nullptr) {
				tile->setHouse(nullptr);
			}
		}
		++tiles_done;
	}

	if (showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void MapProcessor::clearModifiedTileState(Editor& editor, bool showdialog) {
	if (showdialog) {
		g_gui.CreateLoadBar("Clearing modified state from all tiles...");
	}

	uint64_t tiles_done = 0;
	for (MapIterator map_iter = editor.map.begin(); map_iter != editor.map.end(); ++map_iter) {
		if (showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(int(tiles_done / double(editor.map.getTileCount()) * 100.0));
		}

		Tile* tile = map_iter->get();
		ASSERT(tile);
		tile->unmodify();
		++tiles_done;
	}

	if (showdialog) {
		g_gui.DestroyLoadBar();
	}
}
