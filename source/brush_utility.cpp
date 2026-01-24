//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "brush_utility.h"
#include "gui.h"
#include "editor.h"
#include "brush.h"
#include "ground_brush.h"
#include "map.h"
#include "tile.h"

bool BrushUtility::processed[BrushUtility::BLOCK_SIZE * BrushUtility::BLOCK_SIZE] = { false };
int BrushUtility::countMaxFills = 0;

void BrushUtility::GetTilesToDraw(int mouse_map_x, int mouse_map_y, int floor, std::vector<Position>* tilestodraw, std::vector<Position>* tilestoborder, bool fill) {
	if (fill) {
		Brush* brush = g_gui.GetCurrentBrush();
		if (!brush || !brush->isGround()) {
			return;
		}

		GroundBrush* newBrush = brush->asGround();
		Position position(mouse_map_x, mouse_map_y, floor);

		Tile* tile = g_gui.GetCurrentMap().getTile(position);
		GroundBrush* oldBrush = nullptr;
		if (tile) {
			oldBrush = tile->getGroundBrush();
		}

		if (oldBrush && oldBrush->getID() == newBrush->getID()) {
			return;
		}

		if ((tile && tile->ground && !oldBrush) || (!tile && oldBrush)) {
			return;
		}

		if (tile && oldBrush) {
			GroundBrush* groundBrush = tile->getGroundBrush();
			if (!groundBrush || groundBrush->getID() != oldBrush->getID()) {
				return;
			}
		}

		std::fill(std::begin(processed), std::end(processed), false);
		countMaxFills = 0;
		FloodFill(&g_gui.GetCurrentMap(), position, BLOCK_SIZE / 2, BLOCK_SIZE / 2, oldBrush, tilestodraw);

	} else {
		int brushSize = g_gui.GetBrushSize();
		int brushShape = g_gui.GetBrushShape();

		for (int y = -brushSize - 1; y <= brushSize + 1; y++) {
			for (int x = -brushSize - 1; x <= brushSize + 1; x++) {
				if (brushShape == BRUSHSHAPE_SQUARE) {
					if (x >= -brushSize && x <= brushSize && y >= -brushSize && y <= brushSize) {
						if (tilestodraw) {
							tilestodraw->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
						}
					}
					if (std::abs(x) - brushSize < 2 && std::abs(y) - brushSize < 2) {
						if (tilestoborder) {
							tilestoborder->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
						}
					}
				} else if (brushShape == BRUSHSHAPE_CIRCLE) {
					double distance = sqrt(double(x * x) + double(y * y));
					if (distance < brushSize + 0.005) {
						if (tilestodraw) {
							tilestodraw->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
						}
					}
					if (std::abs(distance - brushSize) < 1.5) {
						if (tilestoborder) {
							tilestoborder->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
						}
					}
				}
			}
		}
	}
}

bool BrushUtility::FloodFill(Map* map, const Position& center, int x, int y, GroundBrush* brush, std::vector<Position>* positions) {
	countMaxFills++;
	if (countMaxFills > (BLOCK_SIZE * 4 * 4)) {
		countMaxFills = 0;
		return true;
	}

	if (x <= 0 || y <= 0 || x >= BLOCK_SIZE || y >= BLOCK_SIZE) {
		return false;
	}

	processed[GetFillIndex(x, y)] = true;

	int px = (center.x + x) - (BLOCK_SIZE / 2);
	int py = (center.y + y) - (BLOCK_SIZE / 2);
	if (px <= 0 || py <= 0 || px >= map->getWidth() || py >= map->getHeight()) {
		return false;
	}

	Tile* tile = map->getTile(px, py, center.z);
	if ((tile && tile->ground && !brush) || (!tile && brush)) {
		return false;
	}

	if (tile && brush) {
		GroundBrush* groundBrush = tile->getGroundBrush();
		if (!groundBrush || groundBrush->getID() != brush->getID()) {
			return false;
		}
	}

	positions->push_back(Position(px, py, center.z));

	bool deny = false;
	if (!processed[GetFillIndex(x - 1, y)]) {
		deny = FloodFill(map, center, x - 1, y, brush, positions);
	}

	if (!deny && !processed[GetFillIndex(x, y - 1)]) {
		deny = FloodFill(map, center, x, y - 1, brush, positions);
	}

	if (!deny && !processed[GetFillIndex(x + 1, y)]) {
		deny = FloodFill(map, center, x + 1, y, brush, positions);
	}

	if (!deny && !processed[GetFillIndex(x, y + 1)]) {
		deny = FloodFill(map, center, x, y + 1, brush, positions);
	}

	return deny;
}
