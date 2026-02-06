#include "ui/replace_tool/replacement_engine.h"

ReplacementEngine::ReplacementEngine() : rng(std::random_device {}()) { }

bool ReplacementEngine::ResolveReplacement(uint16_t& resultId, const ReplacementRule& rule) {
	if (rule.targets.empty()) {
		return false;
	}

	// According to requirement: Roll for first, if fail, roll for second, etc.
	// This is slightly different from a proportional distribution but follows the requested logic.
	// Note: This means subsequent targets have a reduced effective probability.
	std::uniform_int_distribution<int> dist(1, 100);

	for (const auto& target : rule.targets) {
		int roll = dist(rng);
		if (roll <= target.probability) {
			resultId = target.id;
			return true;
		}
	}

	return false;
}

#include "editor/editor.h"
#include "ui/gui.h"
#include <map>

void ReplacementEngine::ExecuteReplacement(Editor* editor, const std::vector<ReplacementRule>& rules) {
	if (rules.empty()) {
		return;
	}

	bool selectionOnly = editor->selection.size() > 0;
	std::map<uint16_t, const ReplacementRule*> ruleMap;
	for (const auto& rule : rules) {
		if (rule.fromId != 0) {
			ruleMap[rule.fromId] = &rule;
		}
	}

	auto finder = [&](Map&, Tile*, Item* item, long long) {
		auto it = ruleMap.find(item->getID());
		if (it != ruleMap.end()) {
			uint16_t newId;
			if (ResolveReplacement(newId, *it->second)) {
				if (newId == TRASH_ITEM_ID) {
					item->setID(0); // Delete/Clear item
				} else {
					item->setID(newId);
				}
			}
		}
	};

	foreach_ItemOnMap(editor->map, finder, selectionOnly);
	editor->map.doChange();
	g_gui.RefreshView();
}
