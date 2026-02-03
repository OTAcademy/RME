#include "app/main.h"
#include "ui/replace_tool/rule_manager.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <wx/stdpaths.h>
#include <wx/dir.h>
#include <wx/filename.h>

// JSON serialization macros
void to_json(nlohmann::json& j, const ReplacementTarget& t) {
	j = nlohmann::json { { "id", t.id }, { "probability", t.probability } };
}
void from_json(const nlohmann::json& j, ReplacementTarget& t) {
	j.at("id").get_to(t.id);
	j.at("probability").get_to(t.probability);
}

void to_json(nlohmann::json& j, const ReplacementRule& r) {
	j = nlohmann::json { { "fromId", r.fromId }, { "targets", r.targets } };
}
void from_json(const nlohmann::json& j, ReplacementRule& r) {
	j.at("fromId").get_to(r.fromId);
	j.at("targets").get_to(r.targets);
}

void to_json(nlohmann::json& j, const RuleSet& s) {
	j = nlohmann::json { { "name", s.name }, { "rules", s.rules } };
}
void from_json(const nlohmann::json& j, RuleSet& s) {
	j.at("name").get_to(s.name);
	j.at("rules").get_to(s.rules);
}

RuleManager& RuleManager::Get() {
	static RuleManager instance;
	return instance;
}

RuleManager::RuleManager() {
	wxFileName::Mkdir(GetRulesDir(), 0777, wxPATH_MKDIR_FULL);
}

std::string RuleManager::GetRulesDir() {
	return (wxStandardPaths::Get().GetUserDataDir() + "\\replacer_rules").ToStdString();
}

bool RuleManager::SaveRuleSet(const RuleSet& ruleSet) {
	std::string path = GetRulesDir() + "\\" + ruleSet.name + ".json";
	std::ofstream file(path);
	if (!file.is_open()) {
		return false;
	}

	nlohmann::json j = ruleSet;
	file << j.dump(4);
	return true;
}

RuleSet RuleManager::LoadRuleSet(const std::string& name) {
	std::string path = GetRulesDir() + "\\" + name + ".json";
	std::ifstream file(path);
	RuleSet ruleSet;
	if (!file.is_open()) {
		return ruleSet;
	}

	try {
		nlohmann::json j;
		file >> j;
		ruleSet = j.get<RuleSet>();
	} catch (...) {
		// Handle error
	}
	return ruleSet;
}

std::vector<std::string> RuleManager::GetAvailableRuleSets() {
	std::vector<std::string> results;
	wxDir dir(GetRulesDir());
	if (!dir.IsOpened()) {
		return results;
	}

	wxString filename;
	bool cont = dir.GetFirst(&filename, "*.json", wxDIR_FILES);
	while (cont) {
		wxFileName fn(filename);
		results.push_back(fn.GetName().ToStdString());
		cont = dir.GetNext(&filename);
	}
	return results;
}
