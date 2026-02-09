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

#ifndef RME_TABLE_BRUSH_H
#define RME_TABLE_BRUSH_H

#include "brushes/brush.h"
#include "brushes/table/table_brush_items.h"

//=============================================================================
// Tablebrush, for tables, and some things that behave like tables
// and with tables I really mean counters.

class TableBrush : public Brush {
public:
	static void init();

	TableBrush();
	~TableBrush() override;

	bool isTable() const override {
		return true;
	}
	TableBrush* asTable() override {
		return static_cast<TableBrush*>(this);
	}

	bool load(pugi::xml_node node, std::vector<std::string>& warnings) override;

	bool canDraw(BaseMap* map, const Position& position) const override;
	void draw(BaseMap* map, Tile* tile, void* parameter) override;
	void undraw(BaseMap* map, Tile* tile) override;
	static void doTables(BaseMap* map, Tile* tile);

	void getRelatedItems(std::vector<uint16_t>& items) override;

	int getLookID() const override {
		return look_id;
	}

	std::string getName() const override {
		return name;
	}
	void setName(const std::string& newName) override {
		name = newName;
	}

	void setLookID(int id) {
		look_id = id;
	}

	bool needBorders() const override {
		return true;
	}

protected:
	TableBrushItems items;

	std::string name;
	uint16_t look_id;

	static uint32_t table_types[256];

	// Friend classes access protected members directly.
	friend class TableBrushLoader;
	friend class TableBorderCalculator;
	friend class LibraryPanel;
};

#endif
