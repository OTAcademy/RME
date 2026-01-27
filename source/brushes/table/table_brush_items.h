//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_BRUSHES_TABLE_TABLE_BRUSH_ITEMS_H
#define RME_BRUSHES_TABLE_TABLE_BRUSH_ITEMS_H

#include <vector>
#include <cstdint>
#include "brushes/brush_enums.h"

// Structs moved from table_brush.h and modernized
struct TableType {
	int chance = 0;
	uint16_t item_id = 0;
};

struct TableNode {
	int total_chance = 0;
	std::vector<TableType> items;
};

class TableBrushItems {
public:
	TableBrushItems() = default;
	~TableBrushItems() = default;

	void addItem(int alignment, uint16_t id, int chance);
	const TableNode& getItems(int alignment) const;

	// Helper to check if any items exist for a specific alignment
	bool hasItems(int alignment) const;

private:
	TableNode table_items[7]; // Using 7 for fixed alignments (0-6)
};

#endif
