//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "brushes/carpet/carpet_brush_items.h"
#include "brushes/brush_utility.h" // For CARPET_CENTER

CarpetBrushItems::CarpetBrushItems() {
	// m_groups is automatically initialized
}

void CarpetBrushItems::addItem(BorderType alignment, int32_t id, int32_t chance) {
	if (alignment < 0 || alignment >= m_groups.size()) {
		return;
	}

	auto& group = m_groups[alignment];
	group.total_chance += chance;
	group.items.push_back({ chance, static_cast<uint16_t>(id) });
}

uint16_t CarpetBrushItems::getRandomItem(BorderType alignment) const {
	if (alignment >= 0 && alignment < m_groups.size()) {
		const auto& group = m_groups[alignment];
		if (group.total_chance > 0) {
			return pickFromGroup(group);
		}
	}

	// Fallback logic from original code:

	// 1. Try Center
	const auto& centerGroup = m_groups[CARPET_CENTER];
	if (alignment != CARPET_CENTER && centerGroup.total_chance > 0) {
		uint16_t id = pickFromGroup(centerGroup);
		if (id != 0) {
			return id;
		}
	}

	// 2. Try any group (Center first, then others)
	// Original code loop: for (int32_t i = 0; i < 12; ++i)
	// Note: Array size is 14, but loop went to 12. We'll stick to 12 to match behavior,
	// or safe iterate all? Original code: `carpet_items[14]` and loop `i < 12`.
	// Let's iterate all valid groups to be safe, but respect original order if possible.

	for (const auto& group : m_groups) {
		if (group.total_chance > 0) {
			uint16_t id = pickFromGroup(group);
			if (id != 0) {
				return id;
			}
		}
	}

	return 0;
}

uint16_t CarpetBrushItems::pickFromGroup(const CarpetGroup& group) {
	if (group.total_chance <= 0 || group.items.empty()) {
		return 0;
	}

	int32_t chance = random(1, group.total_chance);
	for (const auto& item : group.items) {
		if (chance <= item.chance) {
			return item.id;
		}
		chance -= item.chance;
	}
	return 0;
}

void CarpetBrushItems::clear() {
	for (auto& group : m_groups) {
		group.items.clear();
		group.total_chance = 0;
	}
}
