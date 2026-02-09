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
#ifndef RME_TILESET_H_
#define RME_TILESET_H_

#include <memory>
#include <unordered_map>
#include <vector>
#include "brushes/brush_enums.h"

class Brushes;
class TilesetCategory {
public:
	TilesetCategory(Tileset& parent, TilesetCategoryType type);
	~TilesetCategory();
	bool isTrivial() const;
	TilesetCategoryType getType() const {
		return type;
	}
	size_t size() const {
		return brushlist.size();
	}
	void loadBrush(pugi::xml_node node, std::vector<std::string>& warnings);
	void clear();
	bool containsBrush(Brush* brush) const;

protected:
	TilesetCategoryType type;

public:
	std::vector<Brush*> brushlist;
	Tileset& tileset;

private:
	TilesetCategory(const TilesetCategory&);
	TilesetCategory operator=(const TilesetCategory&);
};
using TilesetCategoryArray = std::vector<std::unique_ptr<TilesetCategory>>;
class Tileset {
public:
	Tileset(Brushes& brushes, const std::string& name);
	~Tileset();
	TilesetCategory* getCategory(TilesetCategoryType type);
	const TilesetCategory* getCategory(TilesetCategoryType type) const;
	void loadCategory(pugi::xml_node node, std::vector<std::string>& warnings);
	void clear();
	bool containsBrush(Brush* brush) const;

public:
	std::string name;
	int16_t previousId;
	TilesetCategoryArray categories;

protected:
	Brushes& brushes;

protected:
	Tileset(const Tileset&);
	Tileset operator=(const Tileset&);
	friend class TilesetCategory;
};
using TilesetContainer = std::unordered_map<std::string, Tileset*>;
// Returns tilesets sorted alphabetically by name for UI display.
std::vector<Tileset*> GetSortedTilesets(const TilesetContainer& tilesets);
#endif
