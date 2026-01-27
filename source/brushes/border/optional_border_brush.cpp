//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/border/optional_border_brush.h"

#include "brushes/ground/ground_brush.h"
#include "game/sprites.h"
#include "map/map.h"
#include "map/tile.h"

#include <array>

OptionalBorderBrush::OptionalBorderBrush() {
	//
}

std::string OptionalBorderBrush::getName() const {
	return "Optional Border Tool";
}

int OptionalBorderBrush::getLookID() const {
	return EDITOR_SPRITE_OPTIONAL_BORDER_TOOL;
}

bool OptionalBorderBrush::canDraw(BaseMap* map, const Position& position) const {
	if (Tile* tile = map->getTile(position)) {
		if (GroundBrush* bb = tile->getGroundBrush()) {
			if (bb->hasOptionalBorder()) {
				return false;
			}
		}
	}

	static constexpr std::array<std::pair<int, int>, 8> offsets = { { { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } } };

	for (const auto& [dx, dy] : offsets) {
		if (Tile* tile = map->getTile(position.x + dx, position.y + dy, position.z)) {
			if (GroundBrush* bb = tile->getGroundBrush()) {
				if (bb->hasOptionalBorder()) {
					return true;
				}
			}
		}
	}

	return false;
}

void OptionalBorderBrush::undraw(BaseMap* /*map*/, Tile* tile) {
	tile->setOptionalBorder(false);
}

void OptionalBorderBrush::draw(BaseMap* /*map*/, Tile* tile, void* /*parameter*/) {
	tile->setOptionalBorder(true);
}
