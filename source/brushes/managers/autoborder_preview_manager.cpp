//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/managers/autoborder_preview_manager.h"
#include "brushes/brush.h"
#include "brushes/managers/brush_manager.h"
#include "map/map.h"
#include "map/tile.h"
#include "map/tile_operations.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "brushes/brush_utility.h"
#include "editor/editor.h"

AutoborderPreviewManager g_autoborder_preview;

AutoborderPreviewManager::AutoborderPreviewManager() :
	preview_buffer_map(std::make_unique<BaseMap>()),
	last_pos(Position(-1, -1, -1)) {
}

AutoborderPreviewManager::~AutoborderPreviewManager() = default;

void AutoborderPreviewManager::Clear() {
	preview_buffer_map->clear();
	last_pos = Position(-1, -1, -1);
}

void AutoborderPreviewManager::Update(Editor& editor, const Position& pos, bool alt_pressed) {
	if (!CheckPreconditions(pos)) {
		return;
	}

	CopyMapArea(editor, pos);

	Brush* brush = g_gui.GetCurrentBrush();
	PositionVector tilestodraw;
	PositionVector tilestoborder;

	// Use BrushUtility to get affected tiles based on shape
	BrushUtility::GetTilesToDraw(pos.x, pos.y, pos.z, &tilestodraw, &tilestoborder);

	SimulateBrush(editor, pos, tilestodraw, alt_pressed);
	ApplyBorders(tilestodraw, tilestoborder);
	PruneUnchanged(editor, pos);
}

bool AutoborderPreviewManager::CheckPreconditions(const Position& pos) {
	Brush* brush = g_gui.GetCurrentBrush();
	if (!g_settings.getInteger(Config::USE_AUTOMAGIC) || !brush || !brush->needBorders()) {
		if (preview_buffer_map->size() > 0) {
			Clear();
		}
		return false;
	}

	last_pos = pos;
	return true;
}

void AutoborderPreviewManager::CopyMapArea(Editor& editor, const Position& pos) {
	preview_buffer_map->clear();

	int brush_size = g_gui.GetBrushSize();
	int range = brush_size + 3; // +3 to cover borders and neighbors of borders

	int start_x = pos.x - range;
	int end_x = pos.x + range;
	int start_y = pos.y - range;
	int end_y = pos.y + range;

	// Copy area from editor map to buffer map
	int z = pos.z;
	for (int y = start_y; y <= end_y; ++y) {
		for (int x = start_x; x <= end_x; ++x) {
			Tile* src_tile = editor.map.getTile(x, y, z);
			if (src_tile) {
				// deeply copies tile and its items to buffer map
				// deepCopy now correctly uses the destination map's TileLocation
				std::unique_ptr<Tile> new_tile = src_tile->deepCopy(*preview_buffer_map);
				preview_buffer_map->setTile(std::move(new_tile));
			}
		}
	}
}

void AutoborderPreviewManager::SimulateBrush(Editor& editor, const Position& pos, const std::vector<Position>& tilestodraw, bool alt_pressed) {
	Brush* brush = g_gui.GetCurrentBrush();
	if (!brush) {
		return;
	}

	bool is_wall = brush->isWall() || brush->isDoor();
	bool is_ground = brush->isGround();

	// Apply draw
	for (const auto& p : tilestodraw) {
		Tile* tile = preview_buffer_map->getTile(p);
		if (!tile) {
			// If tile didn't exist in source, we need to create it in buffer
			tile = preview_buffer_map->createTile(p.x, p.y, p.z);
		}

		// Safety check: tile should always have a valid location
		ASSERT(tile->getLocation() != nullptr);

		if (is_wall) {
			TileOperations::cleanWalls(tile, false);
		} else if (is_ground) {
			TileOperations::cleanBorders(tile);
		}

		// Draw the brush
		// Handle ALT key for ground brushes (replace mode)
		if (is_ground && alt_pressed) {
			if (editor.replace_brush) {
				std::pair<bool, GroundBrush*> param(false, editor.replace_brush);
				brush->draw(preview_buffer_map.get(), tile, &param);
			} else {
				std::pair<bool, GroundBrush*> param(true, nullptr);
				brush->draw(preview_buffer_map.get(), tile, &param);
			}
		} else {
			brush->draw(preview_buffer_map.get(), tile, nullptr);
		}
	}
}

void AutoborderPreviewManager::ApplyBorders(const std::vector<Position>& tilestodraw, const std::vector<Position>& tilestoborder) {
	Brush* brush = g_gui.GetCurrentBrush();
	if (!brush) {
		return;
	}

	bool is_eraser = brush->isEraser();
	bool is_wall = brush->isWall() || brush->isDoor();
	bool is_table = brush->isTable();
	bool is_carpet = brush->isCarpet();

	auto process_tile = [&](const Position& p) {
		Tile* tile = preview_buffer_map->getTile(p);
		if (tile) {
			// Safety check: tile should always have a valid location
			ASSERT(tile->getLocation() != nullptr);

			if (is_eraser) {
				TileOperations::wallize(tile, preview_buffer_map.get());
				TileOperations::tableize(tile, preview_buffer_map.get());
				TileOperations::carpetize(tile, preview_buffer_map.get());
				TileOperations::borderize(tile, preview_buffer_map.get());
			} else if (is_wall) {
				TileOperations::wallize(tile, preview_buffer_map.get());
			} else if (is_table) {
				TileOperations::tableize(tile, preview_buffer_map.get());
			} else if (is_carpet) {
				TileOperations::carpetize(tile, preview_buffer_map.get());
			} else {
				// Default/Ground
				TileOperations::borderize(tile, preview_buffer_map.get());
			}
		}
	};

	for (const auto& p : tilestoborder) {
		process_tile(p);
	}

	for (const auto& p : tilestodraw) {
		process_tile(p);
	}
}

void AutoborderPreviewManager::PruneUnchanged(Editor& editor, const Position& pos) {
	int brush_size = g_gui.GetBrushSize();
	int range = brush_size + 3;

	int start_x = pos.x - range;
	int end_x = pos.x + range;
	int start_y = pos.y - range;
	int end_y = pos.y + range;

	// Prune tiles that haven't changed to avoid "shade" overlay
	// Prune tiles that haven't changed to avoid "shade" overlay
	int z = pos.z;
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
			bool equal = buf_tile->isContentEqual(src_tile);

			if (equal) {
				// Remove unmodified tile from buffer to prevent ghosting
				preview_buffer_map->setTile(x, y, z, std::unique_ptr<Tile>());
			}
		}
	}
}
