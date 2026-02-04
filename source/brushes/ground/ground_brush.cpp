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

#include <algorithm>
#include "brushes/ground/ground_brush.h"
#include "brushes/ground/auto_border.h"
#include "brushes/ground/ground_brush_loader.h"
#include "brushes/ground/ground_border_calculator.h"
#include "game/items.h"
#include "map/basemap.h"

uint32_t GroundBrush::border_types[256];

GroundBrush::GroundBrush() :
	z_order(0),
	has_zilch_outer_border(false),
	has_zilch_inner_border(false),
	has_outer_border(false),
	has_inner_border(false),
	optional_border(nullptr),
	use_only_optional(false),
	randomize(true),
	total_chance(0) {
	////
}

GroundBrush::~GroundBrush() {
	for (BorderBlock* borderBlock : borders) {
		if (borderBlock->autoborder) {
			for (SpecificCaseBlock* specificCaseBlock : borderBlock->specific_cases) {
				delete specificCaseBlock;
			}

			if (borderBlock->autoborder->ground) {
				delete borderBlock->autoborder;
			}
		}
		delete borderBlock;
	}
	borders.clear();
}

bool GroundBrush::load(pugi::xml_node node, wxArrayString& warnings) {
	return GroundBrushLoader::load(*this, node, warnings);
}

void GroundBrush::undraw(BaseMap* map, Tile* tile) {
	ASSERT(tile);
	if (tile->hasGround() && tile->ground->getGroundBrush() == this) {
		delete tile->ground;
		tile->ground = nullptr;
	}
}

void GroundBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);
	if (border_items.empty()) {
		return;
	}

	if (parameter != nullptr) {
		std::pair<bool, GroundBrush*>& param = *reinterpret_cast<std::pair<bool, GroundBrush*>*>(parameter);
		GroundBrush* other = tile->getGroundBrush();
		if (param.first) { // Volatile? :)
			if (other != nullptr) {
				return;
			}
		} else if (other != param.second) {
			return;
		}
	}
	int chance = random(1, total_chance);
	uint16_t id = 0;
	for (const auto& item_block : border_items) {
		if (chance < item_block.chance) {
			id = item_block.id;
			break;
		}
	}
	if (id == 0) {
		id = border_items.front().id;
	}

	tile->addItem(Item::Create(id));
}

const GroundBrush::BorderBlock* GroundBrush::getBrushTo(GroundBrush* first, GroundBrush* second) {
	if (first) {
		if (second) {
			if (first->getZ() < second->getZ() && second->hasOuterBorder()) {
				if (first->hasInnerBorder()) {
					for (BorderBlock* bb : first->borders) {
						if (bb->outer) {
							continue;
						} else if (bb->to == second->getID() || bb->to == 0xFFFFFFFF) {
							return bb;
						}
					}
				}
				for (BorderBlock* bb : second->borders) {
					if (!bb->outer) {
						continue;
					} else if (bb->to == first->getID()) {
						return bb;
					} else if (bb->to == 0xFFFFFFFF) {
						return bb;
					}
				}
			} else if (first->hasInnerBorder()) {
				for (BorderBlock* bb : first->borders) {
					if (bb->outer) {
						continue;
					} else if (bb->to == second->getID()) {
						return bb;
					} else if (bb->to == 0xFFFFFFFF) {
						return bb;
					}
				}
			}
		} else if (first->hasInnerZilchBorder()) {
			for (BorderBlock* bb : first->borders) {
				if (bb->outer) {
					continue;
				} else if (bb->to == 0) {
					return bb;
				}
			}
		}
	} else if (second && second->hasOuterZilchBorder()) {
		for (BorderBlock* bb : second->borders) {
			if (!bb->outer) {
				continue;
			} else if (bb->to == 0) {
				return bb;
			}
		}
	}
	return nullptr;
}

inline GroundBrush* extractGroundBrushFromTile(BaseMap* map, uint32_t x, uint32_t y, uint32_t z) {
	Tile* t = map->getTile(x, y, z);
	return t ? t->getGroundBrush() : nullptr;
}

void GroundBrush::doBorders(BaseMap* map, Tile* tile) {
	GroundBorderCalculator::calculate(map, tile);
}
void GroundBrush::getRelatedItems(std::vector<uint16_t>& items) {
	for (const auto& item_block : border_items) {
		if (item_block.id != 0) {
			items.push_back(item_block.id);
		}
	}

	for (const BorderBlock* bb : borders) {
		if (bb->autoborder) {
			for (uint32_t tile_id : bb->autoborder->tiles) {
				if (tile_id != 0) {
					items.push_back(static_cast<uint16_t>(tile_id));
				}
			}
		}
		for (const SpecificCaseBlock* sc : bb->specific_cases) {
			if (sc->to_replace_id != 0) {
				items.push_back(sc->to_replace_id);
			}
			if (sc->with_id != 0) {
				items.push_back(sc->with_id);
			}
		}
	}
}
