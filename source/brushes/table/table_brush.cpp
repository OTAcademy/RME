//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "brushes/table/table_brush.h"
#include "brushes/table/table_brush_items.h"
#include "brushes/table/table_brush_loader.h"
#include "brushes/table/table_border_calculator.h"

#include "game/items.h"
#include "map/basemap.h"

uint32_t TableBrush::table_types[256];

//=============================================================================
// Table brush

TableBrush::TableBrush() :
	look_id(0) {
	////
}

TableBrush::~TableBrush() {
	////
}

bool TableBrush::load(pugi::xml_node node, wxArrayString& warnings) {
	return TableBrushLoader::load(node, *this, items, warnings);
}

bool TableBrush::canDraw(BaseMap* map, const Position& position) const {
	return true;
}

void TableBrush::undraw(BaseMap* map, Tile* t) {
	std::erase_if(t->items, [this](Item* item) {
		if (item->isTable() && item->getTableBrush() == this) {
			delete item;
			return true;
		}
		return false;
	});
}

void TableBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	undraw(map, tile); // Remove old

	const TableNode& tn = items.getItems(0);
	if (tn.total_chance <= 0) {
		return;
	}
	int chance = random(1, tn.total_chance);
	uint16_t type = 0;

	for (const auto& item : tn.items) {
		if (chance <= item.chance) {
			type = item.item_id;
			break;
		}
		chance -= item.chance;
	}

	if (type != 0) {
		tile->addItem(Item::Create(type));
	}
}

void TableBrush::doTables(BaseMap* map, Tile* tile) {
	TableBorderCalculator::doTables(map, tile);
}
