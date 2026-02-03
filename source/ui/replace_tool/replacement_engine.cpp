#include "ui/replace_tool/replacement_engine.h"

ReplacementEngine::ReplacementEngine() : rng(std::random_device {}()) { }

bool ReplacementEngine::ResolveReplacement(uint16_t& resultId, const ReplacementRule& rule) {
	if (rule.targets.empty()) {
		return false;
	}

	// According to requirement: Roll for first, if fail, roll for second, etc.
	// This is slightly different from a proportional distribution but follows the requested logic.
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
