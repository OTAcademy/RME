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

void ReplacementEngine::ExecuteReplacement(Editor* editor, const std::vector<ReplacementRule>& rules, ReplaceScope scope, const std::vector<Position>* posVec) {
	if (rules.empty()) {
		return;
	}

	std::map<uint16_t, const ReplacementRule*> ruleMap;
	for (const auto& rule : rules) {
		if (rule.fromId != 0) {
			ruleMap[rule.fromId] = &rule;
		}
	}

	auto tileProcessor = [this, &ruleMap, editor](Tile* tile) {
		auto finder = [this, &ruleMap](Map&, Tile*, Item* item, long long) {
			auto it = ruleMap.find(item->getID());
			if (it != ruleMap.end()) {
				uint16_t newId;
				if (ResolveReplacement(newId, *it->second)) {
					if (newId == TRASH_ITEM_ID) {
						item->setID(0);
					} else {
						item->setID(newId);
					}
				}
			}
		};

		long long dummy = 0;
		if (tile->ground) {
			finder(editor->map, tile, tile->ground, ++dummy);
		}

		std::vector<Container*> containers;
		for (auto* item : tile->items) {
			containers.clear();
			Container* container = item->asContainer();
			finder(editor->map, tile, item, ++dummy);

			if (container) {
				containers.push_back(container);
				size_t index = 0;
				while (index < containers.size()) {
					container = containers[index++];
					ItemVector& v = container->getVector();
					for (auto* i : v) {
						Container* c = i->asContainer();
						finder(editor->map, tile, i, ++dummy);
						if (c) {
							containers.push_back(c);
						}
					}
				}
			}
		}
	};

	if (scope == ReplaceScope::Viewport && posVec) {
		// Use Viewport scope
		for (const auto& pos : *posVec) {
			Tile* tile = editor->map.getTile(pos);
			if (tile) {
				tileProcessor(tile);
			}
		}
	} else if (scope == ReplaceScope::Selection && !editor->selection.empty()) {
		// Use Selection scope
		for (Tile* tile : editor->selection.getTiles()) {
			tileProcessor(tile);
		}
	} else if (scope == ReplaceScope::AllMap) {
		// Use All Map scope
		auto globalFinder = [this, &ruleMap](Map&, Tile*, Item* item, long long) {
			auto it = ruleMap.find(item->getID());
			if (it != ruleMap.end()) {
				uint16_t newId;
				if (ResolveReplacement(newId, *it->second)) {
					if (newId == TRASH_ITEM_ID) {
						item->setID(0);
					} else {
						item->setID(newId);
					}
				}
			}
		};
		foreach_ItemOnMap(editor->map, globalFinder, false);
	}

	editor->map.doChange();
	g_gui.RefreshView();
}
