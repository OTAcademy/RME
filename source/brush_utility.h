//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_BRUSH_UTILITY_H_
#define RME_BRUSH_UTILITY_H_

#include "position.h"
#include <vector>

class Map;
class GroundBrush;

class BrushUtility {
public:
	static void GetTilesToDraw(int mouse_map_x, int mouse_map_y, int floor, std::vector<Position>* tilestodraw, std::vector<Position>* tilestoborder, bool fill = false);

private:
	static bool FloodFill(Map* map, const Position& center, int x, int y, GroundBrush* brush, std::vector<Position>* positions);

	enum {
		BLOCK_SIZE = 100
	};

	static inline int GetFillIndex(int x, int y) {
		return x + BLOCK_SIZE * y;
	}

	static bool processed[BLOCK_SIZE * BLOCK_SIZE];
	static int countMaxFills;
};

#endif
