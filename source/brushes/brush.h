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

#ifndef RME_BRUSH_H_
#define RME_BRUSH_H_
#include "app/main.h"

#include "map/position.h"

#include "brushes/brush_enums.h"
#include <map>
#include <string>
#include <memory>
#include <string_view>

// Thanks to a million forward declarations, we don't have to include any files!
// TODO move to a declarations file.
class ItemType;
class Sprite;
class CreatureType;
class BaseMap;
class House;
class Item;
class Tile;
using TileVector = std::vector<Tile*>;
class AutoBorder;
class Brush;
class RAWBrush;
class DoodadBrush;
class TerrainBrush;
class GroundBrush;
class WallBrush;
class WallDecorationBrush;
class TableBrush;
class CarpetBrush;
class DoorBrush;
class OptionalBorderBrush;
class CreatureBrush;
class SpawnBrush;
class HouseBrush;
class HouseExitBrush;
class WaypointBrush;
class FlagBrush;
class EraserBrush;

//=============================================================================
// Brushes, holds all brushes

using BrushMap = std::multimap<std::string, std::unique_ptr<Brush>, std::less<>>;

class Brushes {
public:
	Brushes();
	~Brushes();

	void init();
	void clear();

	Brush* getBrush(std::string_view name) const;

	void addBrush(Brush* brush);

	bool unserializeBorder(pugi::xml_node node, std::vector<std::string>& warnings);
	bool unserializeBrush(pugi::xml_node node, std::vector<std::string>& warnings);

	const BrushMap& getMap() const {
		return brushes;
	}

protected:
	using BorderMap = std::map<uint32_t, std::unique_ptr<AutoBorder>>;
	BrushMap brushes;
	BorderMap borders;

	friend class AutoBorder;
	friend class GroundBrush;
	friend class GroundBrushLoader;
};

extern Brushes g_brushes;

//=============================================================================
// Common brush interface

class Brush {
public:
	Brush();
	virtual ~Brush();

	virtual bool load(pugi::xml_node node, std::vector<std::string>& warnings) {
		return true;
	}

	virtual void draw(BaseMap* map, Tile* tile, void* parameter = nullptr) = 0;
	virtual void undraw(BaseMap* map, Tile* tile) = 0;
	virtual bool canDraw(BaseMap* map, const Position& position) const = 0;

	//
	uint32_t getID() const {
		return id;
	}

	virtual std::string getName() const = 0;
	virtual void setName(const std::string& newName) {
		ASSERT(_MSG("setName attempted on nameless brush!"));
	}

	virtual int getLookID() const = 0;
	virtual Sprite* getSprite() const {
		return nullptr;
	}

	virtual bool needBorders() const {
		return false;
	}

	virtual bool canDrag() const {
		return false;
	}
	virtual bool canSmear() const {
		return true;
	}

	virtual bool oneSizeFitsAll() const {
		return false;
	}

	virtual int32_t getMaxVariation() const {
		return 0;
	}

	virtual bool isRaw() const {
		return false;
	}
	virtual bool isDoodad() const {
		return false;
	}
	virtual bool isTerrain() const {
		return false;
	}
	virtual bool isGround() const {
		return false;
	}
	virtual bool isWall() const {
		return false;
	}
	virtual bool isWallDecoration() const {
		return false;
	}
	virtual bool isTable() const {
		return false;
	}
	virtual bool isCarpet() const {
		return false;
	}
	virtual bool isDoor() const {
		return false;
	}
	virtual bool isOptionalBorder() const {
		return false;
	}
	virtual bool isCreature() const {
		return false;
	}
	virtual bool isSpawn() const {
		return false;
	}
	virtual bool isHouse() const {
		return false;
	}
	virtual bool isHouseExit() const {
		return false;
	}
	virtual bool isWaypoint() const {
		return false;
	}
	virtual bool isFlag() const {
		return false;
	}
	virtual bool isEraser() const {
		return false;
	}

	virtual RAWBrush* asRaw() {
		return nullptr;
	}
	virtual DoodadBrush* asDoodad() {
		return nullptr;
	}
	virtual TerrainBrush* asTerrain() {
		return nullptr;
	}
	virtual GroundBrush* asGround() {
		return nullptr;
	}
	virtual WallBrush* asWall() {
		return nullptr;
	}
	virtual WallDecorationBrush* asWallDecoration() {
		return nullptr;
	}
	virtual TableBrush* asTable() {
		return nullptr;
	}
	virtual CarpetBrush* asCarpet() {
		return nullptr;
	}
	virtual DoorBrush* asDoor() {
		return nullptr;
	}
	virtual OptionalBorderBrush* asOptionalBorder() {
		return nullptr;
	}
	virtual CreatureBrush* asCreature() {
		return nullptr;
	}
	virtual SpawnBrush* asSpawn() {
		return nullptr;
	}
	virtual HouseBrush* asHouse() {
		return nullptr;
	}
	virtual HouseExitBrush* asHouseExit() {
		return nullptr;
	}
	virtual WaypointBrush* asWaypoint() {
		return nullptr;
	}
	virtual FlagBrush* asFlag() {
		return nullptr;
	}
	virtual EraserBrush* asEraser() {
		return nullptr;
	}

	bool visibleInPalette() const {
		return visible;
	}
	void flagAsVisible() {
		visible = true;
	}

	bool hasCollection() const {
		return usesCollection;
	}
	void setCollection() {
		usesCollection = true;
	}

protected:
	static uint32_t id_counter;
	uint32_t id;
	bool visible; // Visible in any palette?
	bool usesCollection;
};

//=============================================================================
// Terrain brush interface

class TerrainBrush : public Brush {
public:
	TerrainBrush();
	~TerrainBrush() override;

	bool isTerrain() const override {
		return true;
	}
	TerrainBrush* asTerrain() override {
		return static_cast<TerrainBrush*>(this);
	}

	bool canDraw(BaseMap* map, const Position& position) const override {
		return true;
	}

	std::string getName() const override {
		return name;
	}
	void setName(const std::string& newName) override {
		name = newName;
	}

	virtual int32_t getZ() const {
		return 0;
	}
	int32_t getLookID() const override {
		return look_id;
	}

	bool needBorders() const override {
		return true;
	}
	bool canDrag() const override {
		return true;
	}

	bool friendOf(TerrainBrush* other);

protected:
	std::vector<uint32_t> friends;
	std::string name;
	uint16_t look_id;
	bool hate_friends;
};

//=============================================================================
// Rampbrush, add ramps to mountains.

class EraserBrush : public Brush {
public:
	EraserBrush();
	~EraserBrush() override;

	bool isEraser() const override {
		return true;
	}
	EraserBrush* asEraser() override {
		return static_cast<EraserBrush*>(this);
	}

	bool canDraw(BaseMap* map, const Position& position) const override;
	void draw(BaseMap* map, Tile* tile, void* parameter) override;
	void undraw(BaseMap* map, Tile* tile) override;

	bool needBorders() const override {
		return true;
	}
	bool canDrag() const override {
		return true;
	}
	int getLookID() const override;
	std::string getName() const override;
};

//=============================================================================

#endif
