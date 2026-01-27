//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_FLAG_BRUSH_H_
#define RME_FLAG_BRUSH_H_

#include "brushes/brush.h"

class FlagBrush : public Brush {
public:
	explicit FlagBrush(uint32_t flag);
	~FlagBrush() override = default;

	bool isFlag() const override {
		return true;
	}
	FlagBrush* asFlag() override {
		return this;
	}

	bool canDraw(BaseMap* map, const Position& position) const override;
	void draw(BaseMap* map, Tile* tile, void* parameter) override;
	void undraw(BaseMap* map, Tile* tile) override;

	bool canDrag() const override {
		return true;
	}
	int getLookID() const override;
	std::string getName() const override;

protected:
	uint32_t flag;
};

#endif
