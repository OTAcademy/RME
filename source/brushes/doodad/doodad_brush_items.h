//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DOODAD_BRUSH_ITEMS_H
#define RME_DOODAD_BRUSH_ITEMS_H

#include "brushes/doodad/doodad_brush_types.h"
#include <vector>
#include <memory>
#include <random>
#include <unordered_set>

class Item;

class DoodadBrushItems {
public:
	struct SingleBlock {
		int chance;
		std::unique_ptr<Item> item;
	};

	struct CompositeBlock {
		int chance;
		CompositeTileList items;
	};

	struct AlternativeBlock {
		AlternativeBlock();
		~AlternativeBlock();

		std::vector<SingleBlock> single_items;
		std::vector<CompositeBlock> composite_items;

		int composite_chance; // Total chance of a composite
		int single_chance; // Total chance of a single object
	};

	DoodadBrushItems();
	~DoodadBrushItems();

	// Creation
	AlternativeBlock* createAlternative();
	void addAlternative(std::unique_ptr<AlternativeBlock> alternative);

	void addSingleToBlock(AlternativeBlock* block, std::unique_ptr<Item> item, int chance);
	void addCompositeToBlock(AlternativeBlock* block, const CompositeTileList& items, int chance);

	// Access
	const CompositeTileList& getComposite(int variation) const;
	bool isEmpty(int variation, const DoodadBrushSettings& settings) const;
	int getCompositeChance(int variation) const;
	int getSingleChance(int variation) const;
	int getTotalChance(int variation) const;

	bool hasSingleObjects(int variation) const;
	bool hasCompositeObjects(int variation) const;

	bool ownsItem(Item* item, const DoodadBrushSettings& brushSettings) const;
	bool ownsItem(uint16_t id) const;

	const std::vector<std::unique_ptr<AlternativeBlock>>& getAlternatives() const {
		return alternatives;
	}

	// Randomization

	Item* getRandomSingleItem(int variation) const;

private:
	std::vector<std::unique_ptr<AlternativeBlock>> alternatives;
	std::unordered_set<uint16_t> owned_items_cache; // O(1) lookups
	// Helper to handle variation wrapping
	const AlternativeBlock* getAlternative(int variation) const;
};

#endif
