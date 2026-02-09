//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_CARPET_BRUSH_ITEMS_H
#define RME_CARPET_BRUSH_ITEMS_H

#include "brushes/brush.h"
#include <vector>
#include <array>

//=============================================================================
// Manages the collection of items for a carpet brush
class CarpetBrushItems {
	// Friend declarations removed for loose coupling

public:
	CarpetBrushItems();

	// Adds a weighted item for a specific alignment
	void addItem(BorderType alignment, int32_t id, int32_t chance);

	// Selects a random item for the given alignment
	uint16_t getRandomItem(BorderType alignment) const;

	// Clears all items
	void clear();

public:
	struct CarpetItem {
		int32_t chance;
		uint16_t id;
	};

	struct CarpetGroup {
		std::vector<CarpetItem> items;
		int32_t total_chance = 0;
	};

	const std::array<CarpetGroup, 14>& getGroups() const {
		return m_groups;
	}

	// Helper to pick from a specific group
	static uint16_t pickFromGroup(const CarpetGroup& group);

	// Using std::array instead of C-style array for better safety
	std::array<CarpetGroup, 14> m_groups;
};

#endif
