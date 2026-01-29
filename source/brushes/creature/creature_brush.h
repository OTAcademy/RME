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

#ifndef RME_CREATURE_BRUSH_H
#define RME_CREATURE_BRUSH_H

#include "brushes/brush.h"

class Sprite;

//=============================================================================
// CreatureBrush, place creatures

class CreatureBrush : public Brush {
public:
	CreatureBrush(CreatureType* type); // Create a RAWBrush of the specified type
	~CreatureBrush() override;

	bool isCreature() const override {
		return true;
	}
	CreatureBrush* asCreature() override {
		return static_cast<CreatureBrush*>(this);
	}

	bool canDraw(BaseMap* map, const Position& position) const override;
	void draw(BaseMap* map, Tile* tile, void* parameter) override;
	void draw_creature(BaseMap* map, Tile* tile);
	void undraw(BaseMap* map, Tile* tile) override;

	CreatureType* getType() const {
		return creature_type;
	}

	int getLookID() const override; // We don't have a look type, this will always return 0
	Sprite* getSprite() const override;
	std::string getName() const override;
	bool canDrag() const override {
		return false;
	}
	bool canSmear() const override {
		return true;
	}
	bool oneSizeFitsAll() const override {
		return true;
	}

protected:
	CreatureType* creature_type;
	mutable std::unique_ptr<Sprite> creature_sprite_wrapper;
};

#endif
