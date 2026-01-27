//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DOOR_BRUSH_H_
#define RME_DOOR_BRUSH_H_

#include "brushes/brush.h"

class DoorBrush : public Brush {
public:
	explicit DoorBrush(DoorType doortype);
	~DoorBrush() override = default;

	bool isDoor() const override {
		return true;
	}
	DoorBrush* asDoor() override {
		return this;
	}

	static void switchDoor(Item* door);

	bool canDraw(BaseMap* map, const Position& position) const override;
	void draw(BaseMap* map, Tile* tile, void* parameter) override;
	void undraw(BaseMap* map, Tile* tile) override;

	int getLookID() const override;
	std::string getName() const override;
	bool oneSizeFitsAll() const override {
		return true;
	}

protected:
	DoorType doortype;
};

#endif
