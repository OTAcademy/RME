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

#ifndef RME_RAW_BRUSH_H
#define RME_RAW_BRUSH_H

#include "brushes/brush.h"

//=============================================================================
// RAWBrush, draw items like SimOne's editor

class RAWBrush : public Brush {
public:
	RAWBrush(uint16_t itemid); // Create a RAWBrush of the specified type
	~RAWBrush() override;

	bool isRaw() const override {
		return true;
	}
	RAWBrush* asRaw() override {
		return static_cast<RAWBrush*>(this);
	}

	bool canDraw(BaseMap* map, const Position& position) const override {
		return true;
	}
	void draw(BaseMap* map, Tile* tile, void* parameter) override;
	void undraw(BaseMap* map, Tile* tile) override;

	bool canDrag() const override {
		return true;
	}
	int getLookID() const override;
	std::string getName() const override;
	ItemType* getItemType() const {
		return itemtype;
	}
	uint16_t getItemID() const;

protected:
	ItemType* itemtype;
};

#endif
