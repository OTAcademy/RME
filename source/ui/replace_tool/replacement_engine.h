#ifndef RME_REPLACEMENT_ENGINE_H_
#define RME_REPLACEMENT_ENGINE_H_

#include "app/main.h"
#include "rule_manager.h"
#include "map/position.h"
#include <random>
#include <vector>

enum class ReplaceScope {
	Selection,
	Viewport,
	AllMap
};

class ReplacementEngine {
public:
	ReplacementEngine();

	// Resolve the 1:N probability rule
	// Returns true if a replacement should happen, resultId contains the new ID
	bool ResolveReplacement(uint16_t& resultId, const ReplacementRule& rule);

	// Execute the replacement on the map
	// If scope is Viewport, posVec must contain the tiles to process.
	void ExecuteReplacement(class Editor* editor, const std::vector<ReplacementRule>& rules, ReplaceScope scope, const std::vector<Position>* posVec = nullptr);

private:
	std::mt19937 rng;
};

#endif
