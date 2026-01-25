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

#ifndef RME_FORWARD_H
#define RME_FORWARD_H

class Map;
class Tile;
class TileLocation;
class Item;
class container;
class Spawn;
struct Outfit;
class House;
class Creature;
class BaseMap;
class Waypoint;
class Waypoints;
class Tileset;
class Town;
class Position;
class Editor;
class GUI;
class Selection;
class CopyBuffer;
class ItemType;
class Creatures;
class ItemDatabase;
class QTreeNode;
class Floor;
class Action;

class Brush;

#include <unordered_set>

using HouseExitList = std::vector<uint32_t>;
using TileVector = std::vector<Tile*>;
using TileSet = std::unordered_set<Tile*>;
using ItemVector = std::vector<Item*>;
using BrushVector = std::vector<Brush*>;

#endif
