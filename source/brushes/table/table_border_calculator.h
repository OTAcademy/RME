//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_BRUSHES_TABLE_TABLE_BORDER_CALCULATOR_H
#define RME_BRUSHES_TABLE_TABLE_BORDER_CALCULATOR_H

#include <cstdint>

class BaseMap;
class Tile;
class TableBrush;

class TableBorderCalculator {
public:
	static void doTables(BaseMap* map, Tile* tile);

private:
	static bool hasMatchingTableBrushAtTile(BaseMap* map, TableBrush* table_brush, int32_t x, int32_t y, int32_t z);
};

#endif
