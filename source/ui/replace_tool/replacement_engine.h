#ifndef RME_REPLACEMENT_ENGINE_H_
#define RME_REPLACEMENT_ENGINE_H_

#include "app/main.h"
#include "ui/replace_tool/rule_manager.h"
#include <random>

class ReplacementEngine {
public:
	ReplacementEngine();

	// Resolve the 1:N probability rule
	// Returns true if a replacement should happen, resultId contains the new ID
	bool ResolveReplacement(uint16_t& resultId, const ReplacementRule& rule);

private:
	std::mt19937 rng;
};

#endif
