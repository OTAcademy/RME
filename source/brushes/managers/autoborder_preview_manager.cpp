//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/managers/autoborder_preview_manager.h"
#include "brushes/brush.h"
#include "brushes/managers/brush_manager.h"
#include "map/map.h"
#include "map/tile.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "brushes/brush_utility.h"

AutoborderPreviewManager g_autoborder_preview;

AutoborderPreviewManager::AutoborderPreviewManager() :
	preview_buffer_map(std::make_unique<BaseMap>()),
	last_pos(Position(-1, -1, -1)),
	last_z(-1) {
}

AutoborderPreviewManager::~AutoborderPreviewManager() = default;

void AutoborderPreviewManager::Clear() {
	preview_buffer_map->clear();
	last_pos = Position(-1, -1, -1);
}

void AutoborderPreviewManager::Update(Editor& editor, const Position& pos) {

	if (!g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		if (preview_buffer_map->size() > 0) {
			Clear();
		}
		return;
	}

	Brush* brush = g_gui.GetCurrentBrush();
	if (!brush || !brush->needBorders()) {
		if (preview_buffer_map->size() > 0) {
			Clear();
		}
		return;
	}

	// Avoid updating if nothing changed (optimization)
	// Actually, we need to update if brush shape/size changed too, but that's harder to track cheaply.
	// We'll rely on pos changing for now, but mouse move usually implies pos might change.
	// Note: 'pos' passed here should be the map position (tile coords).

	if (pos == last_pos && brush->isGround()) {
		// If position is same, check if we really need to update?
		// Brush settings might have changed. For now, let's update always or check simple things.
		// Safe to re-update.
	}
	last_pos = pos;

	preview_buffer_map->clear();

	int brush_size = g_gui.GetBrushSize();
	int range = brush_size + 3; // +3 to cover borders and neighbors of borders

	int start_x = pos.x - range;
	int end_x = pos.x + range;
	int start_y = pos.y - range;
	int end_y = pos.y + range;

	// Copy area from editor map to buffer map
	for (int z = pos.z; z <= pos.z; ++z) { // usually we only affect current Z
		for (int y = start_y; y <= end_y; ++y) {
			for (int x = start_x; x <= end_x; ++x) {
				Tile* src_tile = editor.map.getTile(x, y, z);
				if (src_tile) {
					// deeply copies tile and its items to buffer map
					Tile* new_tile = src_tile->deepCopy(*preview_buffer_map);
					preview_buffer_map->setTile(new_tile);
				}
			}
		}
	}

	// Now apply the brush operation on the buffer map
	// We need to simulate the draw operation.
	// Similar to DrawingController::HandleClick/Drag logic for drawing.

	bool is_wall = brush->isWall() || brush->isDoor();
	bool is_ground = brush->isGround();
	bool is_eraser = brush->isEraser();
	bool is_table = brush->isTable();
	bool is_carpet = brush->isCarpet();

	PositionVector tilestodraw;
	PositionVector tilestoborder;

	// Use BrushUtility to get affected tiles based on shape
	// Warning: BrushUtility might use g_gui state (brush size/shape).
	BrushUtility::GetTilesToDraw(pos.x, pos.y, pos.z, &tilestodraw, &tilestoborder);

	// Modify 'tilestodraw' to point to buffer map coordinates (which are same as absolute, fine)
	// But we need to use tiles from buffer_map.

	// Apply draw
	for (const auto& p : tilestodraw) {
		Tile* tile = preview_buffer_map->getTile(p);
		if (!tile) {
			// If tile didn't exist in source, we need to create it in buffer
			TileLocation* loc = preview_buffer_map->createTileL(p);
			tile = preview_buffer_map->allocator(loc);
		}

		if (is_wall) {
			tile->cleanWalls(false);
		} else if (is_ground) {
			tile->cleanBorders();
		}

		// Draw the brush
		// Note: draw might require 'alt' state. We assume standard draw for preview (no alt).
		// If user holds alt, the preview might be wrong if we don't check it.
		// For now assume normal draw.
		bool alt = wxGetKeyState(WXK_ALT); // This is a bit hacky to check here, but effective.

		brush->draw(preview_buffer_map.get(), tile, nullptr);
	}

	// Apply borderize
	// We borderize 'tilestoborder' AND 'tilestodraw' (because they might need re-bordering)
	// DrawingController logic:
	// 1. cleanBorders on tilestodraw
	// 2. draw on tilestodraw
	// 3. borderize tilestoborder (neighbors)
	// 4. borderize tilestodraw (new tiles)

	// In the loop above we cleaned and drew.

	// Now borderize neighbor tiles
	// Robust autoborder logic
	auto process_tile = [&](const Position& p) {
		Tile* tile = preview_buffer_map->getTile(p);
		if (tile) {
			if (is_eraser) {
				tile->wallize(preview_buffer_map.get());
				tile->tableize(preview_buffer_map.get());
				tile->carpetize(preview_buffer_map.get());
				tile->borderize(preview_buffer_map.get());
			} else if (is_wall) {
				tile->wallize(preview_buffer_map.get());
			} else if (is_table) {
				tile->tableize(preview_buffer_map.get());
			} else if (is_carpet) {
				tile->carpetize(preview_buffer_map.get());
			} else {
				// Default/Ground
				tile->borderize(preview_buffer_map.get());
			}
		}
	};

	for (const auto& p : tilestoborder) {
		process_tile(p);
	}

	for (const auto& p : tilestodraw) {
		process_tile(p);
	}

	// Prune tiles that haven't changed to avoid "shade" overlay
	for (int z = pos.z; z <= pos.z; ++z) {
		for (int y = start_y; y <= end_y; ++y) {
			for (int x = start_x; x <= end_x; ++x) {
				Tile* buf_tile = preview_buffer_map->getTile(x, y, z);
				if (!buf_tile) {
					continue;
				}

				Tile* src_tile = editor.map.getTile(x, y, z);
				if (!src_tile) {
					continue;
				}

				// Compare tiles
				bool equal = true;

				// Compare ground
				if (buf_tile->ground != nullptr && src_tile->ground != nullptr) {
					if (buf_tile->ground->getID() != src_tile->ground->getID() || buf_tile->ground->getSubtype() != src_tile->ground->getSubtype()) {
						equal = false;
					}
				} else if (buf_tile->ground != src_tile->ground) {
					equal = false;
				}

				if (equal) {
					// Compare items
					if (buf_tile->items.size() != src_tile->items.size()) {
						equal = false;
					} else {
						for (size_t i = 0; i < buf_tile->items.size(); ++i) {
							Item* it1 = buf_tile->items[i];
							Item* it2 = src_tile->items[i];
							if (it1->getID() != it2->getID() || it1->getSubtype() != it2->getSubtype()) {
								equal = false;
								break;
							}
						}
					}
				}

				if (equal) {
					// Remove unmodified tile from buffer to prevent ghosting
					preview_buffer_map->setTile(x, y, z, nullptr, true);
				}
			}
		}
	}
}
