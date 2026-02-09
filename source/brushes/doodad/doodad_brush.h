//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DOODAD_BRUSH_H
#define RME_DOODAD_BRUSH_H

#include "brushes/brush.h"
#include "brushes/doodad/doodad_brush_items.h"
#include "brushes/doodad/doodad_brush_types.h"

//=============================================================================
// Doodadbrush, add doodads!

class DoodadBrush : public Brush {

public:
	DoodadBrush();
	~DoodadBrush() override;

	bool isDoodad() const override {
		return true;
	}
	DoodadBrush* asDoodad() override {
		return static_cast<DoodadBrush*>(this);
	}

public:
	bool load(pugi::xml_node node, std::vector<std::string>& warnings) override;

	bool canDraw(BaseMap* map, const Position& position) const override {
		return true;
	}
	void draw(BaseMap* map, Tile* tile, void* parameter) override;
	const CompositeTileList& getComposite(int variation) const;
	void undraw(BaseMap* map, Tile* tile) override;

	bool ownsItem(Item* item) const;

	bool isEmpty(int variation) const;

	int getThickness() const {
		return settings.thickness;
	}
	int getThicknessCeiling() const {
		return settings.thickness_ceiling;
	}

	int getCompositeChance(int variation) const;
	int getSingleChance(int variation) const;
	int getTotalChance(int variation) const;

	bool hasSingleObjects(int variation) const;
	bool hasCompositeObjects(int variation) const;

	bool placeOnBlocking() const {
		return settings.on_blocking;
	}
	bool placeOnDuplicate() const {
		return settings.on_duplicate;
	}
	bool doNewBorders() const {
		return settings.do_new_borders;
	}
	void getRelatedItems(std::vector<uint16_t>& items) override;

	bool canSmear() const override {
		return settings.draggable;
	}
	bool canDrag() const override {
		return false;
	}
	bool oneSizeFitsAll() const override {
		return settings.one_size;
	}
	int getLookID() const override {
		return settings.look_id;
	}
	int getMaxVariation() const override {
		return items.getAlternatives().size();
	}
	std::string getName() const override {
		return name;
	}
	void setName(const std::string& newName) override {
		name = newName;
	}

protected:
	std::string name;
	DoodadBrushSettings settings;
	DoodadBrushItems items;
};

#endif
