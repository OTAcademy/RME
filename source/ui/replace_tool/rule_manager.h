#ifndef RME_RULE_MANAGER_H_
#define RME_RULE_MANAGER_H_

#include "app/main.h"
#include <vector>
#include <string>

struct ReplacementTarget {
	uint16_t id;
	int probability; // 0-100
};

struct ReplacementRule {
	uint16_t fromId;
	std::vector<ReplacementTarget> targets;
};

struct RuleSet {
	std::string name;
	std::vector<ReplacementRule> rules;
};

class RuleManager {
public:
	static RuleManager& Get();

	bool SaveRuleSet(const RuleSet& ruleSet);
	RuleSet LoadRuleSet(const std::string& name);
	std::vector<std::string> GetAvailableRuleSets();

private:
	RuleManager();
	std::string GetRulesDir();
};

#endif
