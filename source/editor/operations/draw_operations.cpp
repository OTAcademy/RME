//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "editor/operations/draw_operations.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "ui/gui.h"
#include "brushes/managers/doodad_preview_manager.h"
#include "brushes/brush.h"
#include "brushes/doodad/doodad_brush.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/house/house_exit_brush.h"
#include "brushes/waypoint/waypoint_brush.h"
#include "brushes/wall/wall_brush.h"
#include "brushes/carpet/carpet_brush.h"
#include "brushes/table/table_brush.h"
#include "brushes/creature/creature_brush.h"
#include "brushes/spawn/spawn_brush.h"
#include "brushes/door/door_brush.h"
#include "map/map.h"
#include "map/map.h"
#include "map/tile.h"
#include "app/settings.h"

void DrawOperations::draw(Editor& editor, Position offset, bool alt, bool dodraw) {
	Brush* brush = g_gui.GetCurrentBrush();
	if (!brush) {
		return;
	}

	if (brush->isDoodad()) {
		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());
		BaseMap* buffer_map = g_doodad_preview.GetBufferMap();

		Position delta_pos = offset - Position(0x8000, 0x8000, 0x8);
		PositionList tilestoborder;

		for (MapIterator it = buffer_map->begin(); it != buffer_map->end(); ++it) {
			Tile* buffer_tile = it->get();
			Position pos = buffer_tile->getPosition() + delta_pos;
			if (!pos.isValid()) {
				continue;
			}

			TileLocation* location = editor.map.createTileL(pos);
			Tile* tile = location->get();
			DoodadBrush* doodad_brush = brush->asDoodad();

			if (doodad_brush->placeOnBlocking() || alt) {
				if (tile) {
					bool place = true;
					if (!doodad_brush->placeOnDuplicate() && !alt) {
						for (ItemVector::const_iterator iter = tile->items.begin(); iter != tile->items.end(); ++iter) {
							if (doodad_brush->ownsItem(*iter)) {
								place = false;
								break;
							}
						}
					}
					if (place) {
						Tile* new_tile = tile->deepCopy(editor.map);
						SelectionOperations::removeDuplicateWalls(buffer_tile, new_tile);
						SelectionOperations::doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
						new_tile->merge(buffer_tile);
						action->addChange(std::make_unique<Change>(new_tile));
					}
				} else {
					Tile* new_tile = editor.map.allocator(location);
					SelectionOperations::removeDuplicateWalls(buffer_tile, new_tile);
					SelectionOperations::doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
					new_tile->merge(buffer_tile);
					action->addChange(std::make_unique<Change>(new_tile));
				}
			} else {
				if (tile && !tile->isBlocking()) {
					bool place = true;
					if (!doodad_brush->placeOnDuplicate() && !alt) {
						for (ItemVector::const_iterator iter = tile->items.begin(); iter != tile->items.end(); ++iter) {
							if (doodad_brush->ownsItem(*iter)) {
								place = false;
								break;
							}
						}
					}
					if (place) {
						Tile* new_tile = tile->deepCopy(editor.map);
						SelectionOperations::removeDuplicateWalls(buffer_tile, new_tile);
						SelectionOperations::doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
						new_tile->merge(buffer_tile);
						action->addChange(std::make_unique<Change>(new_tile));
					}
				}
			}
		}
		batch->addAndCommitAction(std::move(action));

		if (!tilestoborder.empty()) {
			action = editor.actionQueue->createAction(batch.get());

			// Remove duplicates
			tilestoborder.sort();
			tilestoborder.unique();

			for (PositionList::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = editor.map.getTile(*it);
				if (tile) {
					Tile* new_tile = tile->deepCopy(editor.map);
					new_tile->borderize(&editor.map);
					new_tile->wallize(&editor.map);
					action->addChange(std::make_unique<Change>(new_tile));
				}
			}
			batch->addAndCommitAction(std::move(action));
		}
		editor.addBatch(std::move(batch), 2);
	} else if (brush->isHouseExit()) {
		HouseExitBrush* house_exit_brush = brush->asHouseExit();
		if (!house_exit_brush->canDraw(&editor.map, offset)) {
			return;
		}

		House* house = editor.map.houses.getHouse(house_exit_brush->getHouseID());
		if (!house) {
			return;
		}

		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());
		action->addChange(std::unique_ptr<Change>(Change::Create(house, offset)));
		batch->addAndCommitAction(std::move(action));
		editor.addBatch(std::move(batch), 2);
	} else if (brush->isWaypoint()) {
		WaypointBrush* waypoint_brush = brush->asWaypoint();
		if (!waypoint_brush->canDraw(&editor.map, offset)) {
			return;
		}

		Waypoint* waypoint = editor.map.waypoints.getWaypoint(waypoint_brush->getWaypoint());
		if (!waypoint || waypoint->pos == offset) {
			return;
		}

		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());
		action->addChange(std::unique_ptr<Change>(Change::Create(waypoint, offset)));
		batch->addAndCommitAction(std::move(action));
		editor.addBatch(std::move(batch), 2);
	} else if (brush->isWall()) {
		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());
		// This will only occur with a size 0, when clicking on a tile (not drawing)
		Tile* tile = editor.map.getTile(offset);
		Tile* new_tile = nullptr;
		if (tile) {
			new_tile = tile->deepCopy(editor.map);
		} else {
			new_tile = editor.map.allocator(editor.map.createTileL(offset));
		}

		if (dodraw) {
			bool b = true;
			brush->asWall()->draw(&editor.map, new_tile, &b);
		} else {
			brush->asWall()->undraw(&editor.map, new_tile);
		}
		action->addChange(std::make_unique<Change>(new_tile));
		batch->addAndCommitAction(std::move(action));
		editor.addBatch(std::move(batch), 2);
	} else if (brush->isSpawn() || brush->isCreature()) {
		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());

		Tile* tile = editor.map.getTile(offset);
		Tile* new_tile = nullptr;
		if (tile) {
			new_tile = tile->deepCopy(editor.map);
		} else {
			new_tile = editor.map.allocator(editor.map.createTileL(offset));
		}
		int param;
		if (!brush->isCreature()) {
			param = g_gui.GetBrushSize();
		}
		if (dodraw) {
			brush->draw(&editor.map, new_tile, &param);
		} else {
			brush->undraw(&editor.map, new_tile);
		}
		action->addChange(std::make_unique<Change>(new_tile));
		batch->addAndCommitAction(std::move(action));
		editor.addBatch(std::move(batch), 2);
	}
}

void DrawOperations::draw(Editor& editor, const PositionVector& tilestodraw, bool alt, bool dodraw) {
	Brush* brush = g_gui.GetCurrentBrush();
	if (!brush) {
		return;
	}

#ifdef __DEBUG__
	if (brush->isGround() || brush->isWall()) {
		// Wrong function, end call
		return;
	}
#endif

	std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_DRAW);

	if (brush->isOptionalBorder()) {
		// We actually need to do borders, but on the same tiles we draw to
		for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = editor.map.createTileL(*it);
			Tile* tile = location->get();
			if (tile) {
				if (dodraw) {
					Tile* new_tile = tile->deepCopy(editor.map);
					brush->draw(&editor.map, new_tile);
					new_tile->borderize(&editor.map);
					action->addChange(std::make_unique<Change>(new_tile));
				} else if (!dodraw && tile->hasOptionalBorder()) {
					Tile* new_tile = tile->deepCopy(editor.map);
					brush->undraw(&editor.map, new_tile);
					new_tile->borderize(&editor.map);
					action->addChange(std::make_unique<Change>(new_tile));
				}
			} else if (dodraw) {
				Tile* new_tile = editor.map.allocator(location);
				brush->draw(&editor.map, new_tile);
				new_tile->borderize(&editor.map);
				if (new_tile->empty()) {
					delete new_tile;
					continue;
				}
				action->addChange(std::make_unique<Change>(new_tile));
			}
		}
	} else {

		for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = editor.map.createTileL(*it);
			Tile* tile = location->get();
			if (tile) {
				Tile* new_tile = tile->deepCopy(editor.map);
				if (dodraw) {
					brush->draw(&editor.map, new_tile, &alt);
				} else {
					brush->undraw(&editor.map, new_tile);
				}
				action->addChange(std::make_unique<Change>(new_tile));
			} else if (dodraw) {
				Tile* new_tile = editor.map.allocator(location);
				brush->draw(&editor.map, new_tile, &alt);
				action->addChange(std::make_unique<Change>(new_tile));
			}
		}
	}
	editor.addAction(std::move(action), 2);
}

void DrawOperations::draw(Editor& editor, const PositionVector& tilestodraw, PositionVector& tilestoborder, bool alt, bool dodraw) {
	Brush* brush = g_gui.GetCurrentBrush();
	if (!brush) {
		return;
	}

	if (brush->isGround() || brush->isEraser()) {
		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());

		for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = editor.map.createTileL(*it);
			Tile* tile = location->get();
			if (tile) {
				Tile* new_tile = tile->deepCopy(editor.map);
				if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
					new_tile->cleanBorders();
				}
				if (dodraw) {
					if (brush->isGround() && alt) {
						std::pair<bool, GroundBrush*> param;
						if (editor.replace_brush) {
							param.first = false;
							param.second = editor.replace_brush;
						} else {
							param.first = true;
							param.second = nullptr;
						}
						g_gui.GetCurrentBrush()->draw(&editor.map, new_tile, &param);
					} else {
						g_gui.GetCurrentBrush()->draw(&editor.map, new_tile, nullptr);
					}
				} else {
					g_gui.GetCurrentBrush()->undraw(&editor.map, new_tile);
					tilestoborder.push_back(*it);
				}
				action->addChange(std::make_unique<Change>(new_tile));
			} else if (dodraw) {
				Tile* new_tile = editor.map.allocator(location);
				if (brush->isGround() && alt) {
					std::pair<bool, GroundBrush*> param;
					if (editor.replace_brush) {
						param.first = false;
						param.second = editor.replace_brush;
					} else {
						param.first = true;
						param.second = nullptr;
					}
					g_gui.GetCurrentBrush()->draw(&editor.map, new_tile, &param);
				} else {
					g_gui.GetCurrentBrush()->draw(&editor.map, new_tile, nullptr);
				}
				action->addChange(std::make_unique<Change>(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(std::move(action));

		if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = editor.actionQueue->createAction(batch.get());
			for (PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = editor.map.createTileL(*it);
				Tile* tile = location->get();
				if (tile) {
					Tile* new_tile = tile->deepCopy(editor.map);
					if (brush->isEraser()) {
						new_tile->wallize(&editor.map);
						new_tile->tableize(&editor.map);
						new_tile->carpetize(&editor.map);
					}
					new_tile->borderize(&editor.map);
					action->addChange(std::make_unique<Change>(new_tile));
				} else {
					Tile* new_tile = editor.map.allocator(location);
					if (brush->isEraser()) {
						// There are no carpets/tables/walls on empty tiles...
						// new_tile->wallize(map);
						// new_tile->tableize(map);
						// new_tile->carpetize(map);
					}
					new_tile->borderize(&editor.map);
					if (!new_tile->empty()) {
						action->addChange(std::make_unique<Change>(new_tile));
					} else {
						delete new_tile;
					}
				}
			}
			batch->addAndCommitAction(std::move(action));
		}

		editor.addBatch(std::move(batch), 2);
	} else if (brush->isTable() || brush->isCarpet()) {
		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());

		for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = editor.map.createTileL(*it);
			Tile* tile = location->get();
			if (tile) {
				Tile* new_tile = tile->deepCopy(editor.map);
				if (dodraw) {
					g_gui.GetCurrentBrush()->draw(&editor.map, new_tile, nullptr);
				} else {
					g_gui.GetCurrentBrush()->undraw(&editor.map, new_tile);
				}
				action->addChange(std::make_unique<Change>(new_tile));
			} else if (dodraw) {
				Tile* new_tile = editor.map.allocator(location);
				g_gui.GetCurrentBrush()->draw(&editor.map, new_tile, nullptr);
				action->addChange(std::make_unique<Change>(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(std::move(action));

		// Do borders!
		action = editor.actionQueue->createAction(batch.get());
		for (PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
			Tile* tile = editor.map.getTile(*it);
			if (brush->isTable()) {
				if (tile && tile->hasTable()) {
					Tile* new_tile = tile->deepCopy(editor.map);
					new_tile->tableize(&editor.map);
					action->addChange(std::make_unique<Change>(new_tile));
				}
			} else if (brush->isCarpet()) {
				if (tile && tile->hasCarpet()) {
					Tile* new_tile = tile->deepCopy(editor.map);
					new_tile->carpetize(&editor.map);
					action->addChange(std::make_unique<Change>(new_tile));
				}
			}
		}
		batch->addAndCommitAction(std::move(action));

		editor.addBatch(std::move(batch), 2);
	} else if (brush->isWall()) {
		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());

		if (alt && dodraw) {
			// This is exempt from USE_AUTOMAGIC
			g_doodad_preview.GetBufferMap()->clear();
			BaseMap* draw_map = g_doodad_preview.GetBufferMap();

			for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				TileLocation* location = editor.map.createTileL(*it);
				Tile* tile = location->get();
				if (tile) {
					Tile* new_tile = tile->deepCopy(editor.map);
					new_tile->cleanWalls(brush->isWall());
					g_gui.GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(*it, new_tile, true);
				} else if (dodraw) {
					Tile* new_tile = editor.map.allocator(location);
					g_gui.GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(*it, new_tile, true);
				}
			}
			for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				// Get the correct tiles from the draw map instead of the editor map
				Tile* tile = draw_map->getTile(*it);
				if (tile) {
					tile->wallize(draw_map);
					action->addChange(std::make_unique<Change>(tile));
				}
			}
			draw_map->clear(false);
			// Commit
			batch->addAndCommitAction(std::move(action));
		} else {
			for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				TileLocation* location = editor.map.createTileL(*it);
				Tile* tile = location->get();
				if (tile) {
					Tile* new_tile = tile->deepCopy(editor.map);
					// Wall cleaning is exempt from automagic
					new_tile->cleanWalls(brush->isWall());
					if (dodraw) {
						g_gui.GetCurrentBrush()->draw(&editor.map, new_tile);
					} else {
						g_gui.GetCurrentBrush()->undraw(&editor.map, new_tile);
					}
					action->addChange(std::make_unique<Change>(new_tile));
				} else if (dodraw) {
					Tile* new_tile = editor.map.allocator(location);
					g_gui.GetCurrentBrush()->draw(&editor.map, new_tile);
					action->addChange(std::make_unique<Change>(new_tile));
				}
			}

			// Commit changes to map
			batch->addAndCommitAction(std::move(action));

			if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				// Do borders!
				action = editor.actionQueue->createAction(batch.get());
				for (PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
					Tile* tile = editor.map.getTile(*it);
					if (tile) {
						Tile* new_tile = tile->deepCopy(editor.map);
						new_tile->wallize(&editor.map);
						// if(*tile == *new_tile) delete new_tile;
						action->addChange(std::make_unique<Change>(new_tile));
					}
				}
				batch->addAndCommitAction(std::move(action));
			}
		}

		editor.actionQueue->addBatch(std::move(batch), 2);
	} else if (brush->isDoor()) {
		std::unique_ptr<BatchAction> batch = editor.actionQueue->createBatch(ACTION_DRAW);
		std::unique_ptr<Action> action = editor.actionQueue->createAction(batch.get());
		DoorBrush* door_brush = brush->asDoor();

		// Loop is kind of redundant since there will only ever be one index.
		for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = editor.map.createTileL(*it);
			Tile* tile = location->get();
			if (tile) {
				Tile* new_tile = tile->deepCopy(editor.map);
				// Wall cleaning is exempt from automagic
				if (brush->isWall()) {
					new_tile->cleanWalls(brush->asWall());
				}
				if (dodraw) {
					door_brush->draw(&editor.map, new_tile, &alt);
				} else {
					door_brush->undraw(&editor.map, new_tile);
				}
				action->addChange(std::make_unique<Change>(new_tile));
			} else if (dodraw) {
				Tile* new_tile = editor.map.allocator(location);
				door_brush->draw(&editor.map, new_tile, &alt);
				action->addChange(std::make_unique<Change>(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(std::move(action));

		if (g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = editor.actionQueue->createAction(batch.get());
			for (PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = editor.map.getTile(*it);
				if (tile) {
					Tile* new_tile = tile->deepCopy(editor.map);
					new_tile->wallize(&editor.map);
					// if(*tile == *new_tile) delete new_tile;
					action->addChange(std::make_unique<Change>(new_tile));
				}
			}
			batch->addAndCommitAction(std::move(action));
		}

		editor.addBatch(std::move(batch), 2);
	} else {
		std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_DRAW);
		for (PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = editor.map.createTileL(*it);
			Tile* tile = location->get();
			if (tile) {
				Tile* new_tile = tile->deepCopy(editor.map);
				if (dodraw) {
					g_gui.GetCurrentBrush()->draw(&editor.map, new_tile);
				} else {
					g_gui.GetCurrentBrush()->undraw(&editor.map, new_tile);
				}
				action->addChange(std::make_unique<Change>(new_tile));
			} else if (dodraw) {
				Tile* new_tile = editor.map.allocator(location);
				g_gui.GetCurrentBrush()->draw(&editor.map, new_tile);
				action->addChange(std::make_unique<Change>(new_tile));
			}
		}
		editor.addAction(std::move(action), 2);
	}
}
