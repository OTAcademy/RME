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

// Helper to validate rule set names
static bool IsValidRuleSetName(const std::string& name) {
	if (name.empty()) {
		return false;
	}
	// Allow alphanumeric, -, _, space
	for (char c : name) {
		if (!isalnum(c) && c != '-' && c != '_' && c != ' ') {
			return false;
		}
	}
	return true;
}

std::string RuleManager::GetRulesDir() {
	wxFileName path(wxStandardPaths::Get().GetUserDataDir(), "replacer_rules");
	return path.GetFullPath().ToStdString();
}

bool RuleManager::SaveRuleSet(const RuleSet& ruleSet) {
	if (!IsValidRuleSetName(ruleSet.name)) {
		wxLogError("Invalid rule set name: %s", ruleSet.name);
		return false;
	}

	wxFileName path(GetRulesDir(), ruleSet.name, "json");
	std::ofstream file(path.GetFullPath().ToStdString());
	if (!file.is_open()) {
		return false;
	}

	nlohmann::json j = ruleSet;
	file << j.dump(4);
	return true;
}

RuleSet RuleManager::LoadRuleSet(const std::string& name) {
	RuleSet ruleSet;
	if (!IsValidRuleSetName(name)) {
		wxLogError("Invalid rule set name: %s", name);
		return ruleSet;
	}

	wxFileName path(GetRulesDir(), name, "json");
	std::ifstream file(path.GetFullPath().ToStdString());
	if (!file.is_open()) {
		return ruleSet;
	}

	try {
		nlohmann::json j;
		file >> j;
		ruleSet = j.get<RuleSet>();
	} catch (const std::exception& e) {
		wxLogError("Failed to load rule set '%s': %s", name, e.what());
	} catch (...) {
		wxLogError("Failed to load rule set '%s' due to an unknown error.", name);
	}
	return ruleSet;
}

bool RuleManager::DeleteRuleSet(const std::string& name) {
	if (!IsValidRuleSetName(name)) {
		wxLogError("Invalid rule set name: %s", name);
		return false;
	}

	wxFileName path(GetRulesDir(), name, "json");
	if (!path.FileExists()) {
		return false;
	}

	return wxRemoveFile(path.GetFullPath());
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
