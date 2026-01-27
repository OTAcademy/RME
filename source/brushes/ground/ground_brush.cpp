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
	for (std::vector<ItemChanceBlock>::const_iterator it = border_items.begin(); it != border_items.end(); ++it) {
		if (chance < it->chance) {
			id = it->id;
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
					for (std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
						BorderBlock* bb = *it;
						if (bb->outer) {
							continue;
						} else if (bb->to == second->getID() || bb->to == 0xFFFFFFFF) {
							return bb;
						}
					}
				}
				for (std::vector<BorderBlock*>::iterator it = second->borders.begin(); it != second->borders.end(); ++it) {
					BorderBlock* bb = *it;
					if (!bb->outer) {
						continue;
					} else if (bb->to == first->getID()) {
						return bb;
					} else if (bb->to == 0xFFFFFFFF) {
						return bb;
					}
				}
			} else if (first->hasInnerBorder()) {
				for (std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
					BorderBlock* bb = *it;
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
			for (std::vector<BorderBlock*>::iterator it = first->borders.begin(); it != first->borders.end(); ++it) {
				BorderBlock* bb = *it;
				if (bb->outer) {
					continue;
				} else if (bb->to == 0) {
					return bb;
				}
			}
		}
	} else if (second && second->hasOuterZilchBorder()) {
		for (std::vector<BorderBlock*>::iterator it = second->borders.begin(); it != second->borders.end(); ++it) {
			BorderBlock* bb = *it;
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
