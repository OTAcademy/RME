//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "brushes/table/table_brush_items.h"
#include "app/main.h"

void TableBrushItems::addItem(int alignment, uint16_t id, int chance) {
	if (alignment < 0 || alignment >= static_cast<int>(table_items.size())) {
		return;
	}

	TableType tt;
	tt.item_id = id;
	tt.chance = chance;

	table_items[alignment].total_chance += tt.chance;
	table_items[alignment].items.push_back(tt);
}

const TableNode& TableBrushItems::getItems(int alignment) const {
	static TableNode emptyNode;
	if (alignment < 0 || alignment >= static_cast<int>(table_items.size())) {
		return emptyNode;
	}
	return table_items[alignment];
}

bool TableBrushItems::hasItems(int alignment) const {
	if (alignment < 0 || alignment >= static_cast<int>(table_items.size())) {
		return false;
	}
	return !table_items[alignment].items.empty();
}
