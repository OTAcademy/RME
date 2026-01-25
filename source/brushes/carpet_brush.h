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

#ifndef RME_CARPET_BRUSH_H
#define RME_CARPET_BRUSH_H

#include "brushes/brush.h"

//=============================================================================
// Carpetbrush, for tables, and some things that behave like tables
// and with tables I really mean counters.

class CarpetBrush : public Brush {
public:
	static void init();

	CarpetBrush();
	~CarpetBrush() override;

	bool isCarpet() const override {
		return true;
	}
	CarpetBrush* asCarpet() override {
		return static_cast<CarpetBrush*>(this);
	}

	bool load(pugi::xml_node node, wxArrayString& warnings) override ;

	bool canDraw(BaseMap* map, const Position& position) const override ;
	void draw(BaseMap* map, Tile* tile, void* parameter) override ;
	void undraw(BaseMap* map, Tile* tile) override ;

	static void doCarpets(BaseMap* map, Tile* tile);

	bool canDrag() const override {
		return true;
	}
	bool needBorders() const override {
		return true;
	}

	int getLookID() const override {
		return look_id;
	}

	std::string getName() const override {
		return name;
	}
	void setName(const std::string& newName) override {
		name = newName;
	}

protected:
	uint16_t getRandomCarpet(BorderType alignment);

	struct CarpetType {
		int32_t chance;
		uint16_t id;
	};

	struct CarpetNode {
		std::vector<CarpetType> items;
		int32_t total_chance;

		CarpetNode() :
			items(), total_chance(0) { }
	};

	CarpetNode carpet_items[14];
	std::string name;
	uint16_t look_id;

	static uint32_t carpet_types[256];
};

#endif
