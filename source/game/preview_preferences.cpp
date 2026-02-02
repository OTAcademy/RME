#include "game/preview_preferences.h"
#include "util/json.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <fstream>
#include <spdlog/spdlog.h>

PreviewPreferences g_preview_preferences;

PreviewPreferences::PreviewPreferences() :
	current_speed(220),
	current_name("You") {
}

PreviewPreferences::~PreviewPreferences() {
}

void to_json(nlohmann::json& j, const Outfit& o) {
	j = nlohmann::json {
		{ "type", o.lookType },
		{ "head", o.lookHead },
		{ "body", o.lookBody },
		{ "legs", o.lookLegs },
		{ "feet", o.lookFeet },
		{ "addons", o.lookAddon },
		{ "mount", o.lookMount }
	};
}

void from_json(const nlohmann::json& j, Outfit& o) {
	o.lookType = j.value("type", 0);
	o.lookHead = j.value("head", 0);
	o.lookBody = j.value("body", 0);
	o.lookLegs = j.value("legs", 0);
	o.lookFeet = j.value("feet", 0);
	o.lookAddon = j.value("addons", 0);
	o.lookMount = j.value("mount", 0);
}

void to_json(nlohmann::json& j, const FavoriteItem& f) {
	j = nlohmann::json {
		{ "label", f.label },
		{ "name", f.name },
		{ "speed", f.speed },
		{ "outfit", f.outfit }
	};
}

void from_json(const nlohmann::json& j, FavoriteItem& f) {
	f.label = j.value("label", "");
	f.name = j.value("name", "");
	f.speed = j.value("speed", 220);
	if (j.contains("outfit")) {
		j.at("outfit").get_to(f.outfit);
	}
}

void PreviewPreferences::load() {
	wxFileName fn(wxStandardPaths::Get().GetUserDataDir(), "preferences.json");
	std::string path = fn.GetFullPath().ToStdString();

	std::ifstream f(path);
	if (!f.is_open()) {
		return;
	}

	try {
		nlohmann::json j;
		f >> j;

		if (j.contains("speed")) {
			current_speed = j["speed"];
		}
		if (j.contains("name")) {
			current_name = j["name"].get<std::string>();
		}

		if (j.contains("last_outfit")) {
			j.at("last_outfit").get_to(current_outfit);
		}
		if (j.contains("favorites")) {
			j.at("favorites").get_to(favorites);
		}
	} catch (const std::exception& e) {
		spdlog::error("Failed to load preview preferences: {}", e.what());
	} catch (...) {
		spdlog::error("An unknown error occurred while loading preview preferences");
	}
}

void PreviewPreferences::save() {
	wxFileName fn(wxStandardPaths::Get().GetUserDataDir(), "preferences.json");
	wxString dir = fn.GetPath();
	if (!wxDirExists(dir)) {
		wxMkdir(dir);
	}

	nlohmann::json j;
	j["speed"] = current_speed;
	j["name"] = current_name;

	j["last_outfit"] = current_outfit;
	j["favorites"] = favorites;

	std::ofstream f(fn.GetFullPath().ToStdString());
	if (f.is_open()) {
		f << j.dump(4);
	}
}
