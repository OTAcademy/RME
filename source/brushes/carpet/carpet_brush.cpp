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

#include "brushes/carpet/carpet_brush.h"
#include "brushes/carpet/carpet_brush_loader.h"
#include "brushes/carpet/carpet_border_calculator.h"
#include "brushes/ground/auto_border.h"
#include "brushes/brush_utility.h"
#include "map/basemap.h"
#include "game/items.h"

//=============================================================================
// Carpet brush

uint32_t CarpetBrush::carpet_types[256];

CarpetBrush::CarpetBrush() :
	look_id(0) {
	////
}

CarpetBrush::~CarpetBrush() {
	////
}

bool CarpetBrush::load(pugi::xml_node node, std::vector<std::string>& warnings) {
	return CarpetBrushLoader::load(*this, node, warnings);
}

bool CarpetBrush::canDraw(BaseMap* map, const Position& position) const {
	return true;
}

void CarpetBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	undraw(map, tile); // Remove old
	tile->addItem(Item::Create(getRandomCarpet(CARPET_CENTER)));
}

void CarpetBrush::undraw(BaseMap* map, Tile* tile) {
	auto& items = tile->items;
	for (auto it = items.begin(); it != items.end();) {
		Item* item = *it;
		if (item->isCarpet()) {
			CarpetBrush* carpetBrush = item->getCarpetBrush();
			if (carpetBrush) {
				delete item;
				it = items.erase(it);
			} else {
				++it;
			}
		} else {
			++it;
		}
	}
}

void CarpetBrush::getRelatedItems(std::vector<uint16_t>& items_out) {
	for (const auto& group : m_items.getGroups()) {
		for (const auto& item : group.items) {
			if (item.id != 0) {
				items_out.push_back(item.id);
			}
		}
	}
}

void CarpetBrush::doCarpets(BaseMap* map, Tile* tile) {
	CarpetBorderCalculator::calculate(map, tile);
}

uint16_t CarpetBrush::getRandomCarpet(BorderType alignment) {
	return m_items.getRandomItem(alignment);
}
