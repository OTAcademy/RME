#include "app/main.h"
#include "replacement_engine.h"

ReplacementEngine::ReplacementEngine() : rng(std::random_device {}()) { }

bool ReplacementEngine::ResolveReplacement(uint16_t& resultId, const ReplacementRule& rule) {
	if (rule.targets.empty()) {
		return false;
	}

	// Calculate total probability weight
	int totalProb = 0;
	for (const auto& target : rule.targets) {
		totalProb += target.probability;
	}

	if (totalProb <= 0) {
		return false;
	}

	// Use a single roll from 1 to max(100, totalProb)
	// This ensures that if the UI says 100% total, exactly one target is picked.
	// If total is < 100, the remainder is "no replacement".
	std::uniform_int_distribution<int> dist(1, std::max(100, totalProb));
	int roll = dist(rng);

	int currentSum = 0;
	for (const auto& target : rule.targets) {
		currentSum += target.probability;
		if (roll <= currentSum) {
			resultId = target.id;
			return true;
		}
	}

	return false;
}

#include "editor/editor.h"
#include "ui/gui.h"
#include "map/tile.h"
#include "game/item.h"
#include <algorithm>
#include <map>
#include <string>

void ReplacementEngine::ExecuteReplacement(Editor* editor, const std::vector<ReplacementRule>& rules) {
	if (rules.empty()) {
		return;
	}

	bool selectionOnly = !editor->selection.empty();
	std::map<uint16_t, const ReplacementRule*> ruleMap;
	for (const auto& rule : rules) {
		if (rule.fromId != 0) {
			ruleMap[rule.fromId] = &rule;
		}
	}

	auto finder = [this, &ruleMap](Map&, Tile*, Item* item, long long) {
		auto it = ruleMap.find(item->getID());
		if (it != ruleMap.end()) {
			uint16_t newId;
			if (ResolveReplacement(newId, *it->second)) {
				if (newId == TRASH_ITEM_ID) {
					item->setID(0); // Mark for deletion/clearing (Note: actual removal might need more logic but this is consistent with previous impl)
				} else {
					item->setID(newId);
				}
			}
		}
	};

	if (selectionOnly) {
		// Optimization: Iterate only over selected tiles instead of the whole map
		long long done = 0;
		std::vector<Container*> containers;
		containers.reserve(32);

		for (Tile* tile : editor->selection.getTiles()) {
			if (tile->ground) {
				finder(editor->map, tile, tile->ground, ++done);
			}

			for (auto* item : tile->items) {
				containers.clear();
				Container* container = item->asContainer();
				finder(editor->map, tile, item, ++done);

				if (container) {
					containers.push_back(container);
					size_t index = 0;
					while (index < containers.size()) {
						container = containers[index++];
						ItemVector& v = container->getVector();
						for (auto* i : v) {
							Container* c = i->asContainer();
							finder(editor->map, tile, i, ++done);
							if (c) {
								containers.push_back(c);
							}
						}
					}
				}
			}
		}
	} else {
		foreach_ItemOnMap(editor->map, finder, false);
	}

	editor->map.doChange();
	g_gui.RefreshView();
}
