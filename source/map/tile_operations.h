//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_TILE_OPERATIONS_H
#define RME_TILE_OPERATIONS_H

class Tile;
class BaseMap;
class WallBrush;

namespace TileOperations {
	void borderize(Tile* tile, BaseMap* map);
	void cleanBorders(Tile* tile);

	void wallize(Tile* tile, BaseMap* map);
	void cleanWalls(Tile* tile, bool dontdelete = false);
	void cleanWalls(Tile* tile, WallBrush* wb);

	void tableize(Tile* tile, BaseMap* map);
	void cleanTables(Tile* tile, bool dontdelete = false);

	void carpetize(Tile* tile, BaseMap* map);
}

#endif
