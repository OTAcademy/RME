//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/doodad/doodad_brush_items.h"
#include "game/item.h"
#include <random>

//=============================================================================
// Doodad Brush Items
//=============================================================================

DoodadBrushItems::AlternativeBlock::AlternativeBlock() :
	composite_chance(0),
	single_chance(0) {
}

DoodadBrushItems::AlternativeBlock::~AlternativeBlock() {
	// std::unique_ptr handles cleanup automatically
}

DoodadBrushItems::DoodadBrushItems() {
}

DoodadBrushItems::~DoodadBrushItems() {
}

DoodadBrushItems::AlternativeBlock* DoodadBrushItems::createAlternative() {
	auto alt = std::make_unique<AlternativeBlock>();
	AlternativeBlock* ptr = alt.get();
	alternatives.push_back(std::move(alt));
	return ptr;
}

void DoodadBrushItems::addAlternative(std::unique_ptr<AlternativeBlock> alternative) {
	// Cache items from this alternative
	for (const auto& single : alternative->single_items) {
		if (single.item) {
			owned_items_cache.insert(single.item->getID());
		}
	}
	for (const auto& composite : alternative->composite_items) {
		for (const auto& pair : composite.items) {
			for (const auto& item : pair.second) {
				if (item) {
					owned_items_cache.insert(item->getID());
				}
			}
		}
	}

	alternatives.push_back(std::move(alternative));
}

void DoodadBrushItems::addSingleToBlock(AlternativeBlock* block, std::unique_ptr<Item> item, int chance) {
	if (!block || !item) {
		return;
	}

	owned_items_cache.insert(item->getID());

	SingleBlock sb;
	sb.item = std::move(item);
	sb.chance = chance;

	block->single_items.push_back(std::move(sb));
	block->single_chance += chance;
}

void DoodadBrushItems::addCompositeToBlock(AlternativeBlock* block, const CompositeTileList& items, int chance) {
	if (!block) {
		return;
	}

	CompositeBlock cb;
	cb.items.reserve(items.size());

	// Deep copy the composite list because we need to own the items
	for (const auto& pair : items) {
		DoodadItemVector copied_items;
		copied_items.reserve(pair.second.size());
		for (const auto& item_ptr : pair.second) {
			if (item_ptr) {
				std::unique_ptr<Item> new_item(item_ptr->deepCopy());
				owned_items_cache.insert(new_item->getID());
				copied_items.push_back(std::move(new_item));
			}
		}
		if (!copied_items.empty()) {
			cb.items.push_back(std::make_pair(pair.first, std::move(copied_items)));
		}
	}

	if (!cb.items.empty()) {
		cb.chance = chance;
		block->composite_items.push_back(std::move(cb));
		block->composite_chance += chance;
	}
}

const DoodadBrushItems::AlternativeBlock* DoodadBrushItems::getAlternative(int variation) const {
	if (alternatives.empty()) {
		return nullptr;
	}
	const size_t n = alternatives.size();
	const size_t index = static_cast<size_t>(variation) % n;
	return alternatives[index].get();
}

const CompositeTileList& DoodadBrushItems::getComposite(int variation) const {
	static CompositeTileList empty;
	const AlternativeBlock* ab = getAlternative(variation);
	if (!ab || ab->composite_items.empty()) {
		return empty;
	}

	int roll = random(1, ab->composite_chance);
	for (const auto& cb : ab->composite_items) {
		if (roll <= cb.chance) {
			return cb.items;
		}
		roll -= cb.chance;
	}
	return empty;
}

bool DoodadBrushItems::isEmpty(int variation, const DoodadBrushSettings& settings) const {
	if (hasCompositeObjects(variation)) {
		return false;
	}
	if (hasSingleObjects(variation)) {
		return false;
	}
	if (settings.thickness > 0) {
		return false;
	}
	return true;
}

int DoodadBrushItems::getCompositeChance(int variation) const {
	const AlternativeBlock* ab = getAlternative(variation);
	return ab ? ab->composite_chance : 0;
}

int DoodadBrushItems::getSingleChance(int variation) const {
	const AlternativeBlock* ab = getAlternative(variation);
	return ab ? ab->single_chance : 0;
}

int DoodadBrushItems::getTotalChance(int variation) const {
	const AlternativeBlock* ab = getAlternative(variation);
	return ab ? (ab->composite_chance + ab->single_chance) : 0;
}

bool DoodadBrushItems::hasSingleObjects(int variation) const {
	const AlternativeBlock* ab = getAlternative(variation);
	return ab && ab->single_chance > 0;
}

bool DoodadBrushItems::hasCompositeObjects(int variation) const {
	const AlternativeBlock* ab = getAlternative(variation);
	return ab && ab->composite_chance > 0;
}

bool DoodadBrushItems::ownsItem(Item* item, const DoodadBrushSettings& brushSettings) const {
	if (!item) {
		return false;
	}
	return ownsItem(item->getID());
}

bool DoodadBrushItems::ownsItem(uint16_t id) const {
	return owned_items_cache.contains(id);
}

Item* DoodadBrushItems::getRandomSingleItem(int variation) const {
	const AlternativeBlock* ab = getAlternative(variation);
	if (!ab || ab->single_items.empty()) {
		return nullptr;
	}

	int roll = random(1, ab->single_chance);
	for (const auto& sb : ab->single_items) {
		if (roll <= sb.chance) {
			return sb.item.get(); // Return raw pointer for use
		}
		roll -= sb.chance;
	}
	return nullptr;
}
