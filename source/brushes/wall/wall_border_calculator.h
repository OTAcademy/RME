//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_WALL_BORDER_CALCULATOR_H
#define RME_WALL_BORDER_CALCULATOR_H

#include <cstdint>

class BaseMap;
class Tile;
class WallBrush;

class WallBorderCalculator {
public:
	static void doWalls(BaseMap* map, Tile* tile);

private:
	static bool hasMatchingWallBrushAtTile(BaseMap* map, WallBrush* wall_brush, int32_t x, int32_t y, int32_t z);
};

#endif
