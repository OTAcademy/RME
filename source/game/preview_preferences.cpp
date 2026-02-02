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
			nlohmann::json lo = j["last_outfit"];
			current_outfit.lookType = lo.value("type", 0);
			current_outfit.lookHead = lo.value("head", 0);
			current_outfit.lookBody = lo.value("body", 0);
			current_outfit.lookLegs = lo.value("legs", 0);
			current_outfit.lookFeet = lo.value("feet", 0);
			current_outfit.lookAddon = lo.value("addons", 0);
			current_outfit.lookMount = lo.value("mount", 0);
		}

		if (j.contains("favorites")) {
			favorites.clear();
			for (auto& fj : j["favorites"]) {
				FavoriteItem item;
				item.label = fj.value("label", "");
				item.name = fj.value("name", "");
				item.speed = fj.value("speed", 220);

				if (fj.contains("outfit")) {
					nlohmann::json o = fj["outfit"];
					item.outfit.lookType = o.value("type", 0);
					item.outfit.lookHead = o.value("head", 0);
					item.outfit.lookBody = o.value("body", 0);
					item.outfit.lookLegs = o.value("legs", 0);
					item.outfit.lookFeet = o.value("feet", 0);
					item.outfit.lookAddon = o.value("addons", 0);
					item.outfit.lookMount = o.value("mount", 0);
				}
				favorites.push_back(item);
			}
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

	j["last_outfit"] = {
		{ "type", current_outfit.lookType },
		{ "head", current_outfit.lookHead },
		{ "body", current_outfit.lookBody },
		{ "legs", current_outfit.lookLegs },
		{ "feet", current_outfit.lookFeet },
		{ "addons", current_outfit.lookAddon },
		{ "mount", current_outfit.lookMount }
	};

	nlohmann::json favs = nlohmann::json::array();
	for (const auto& fav : favorites) {
		favs.push_back({ { "label", fav.label }, { "name", fav.name }, { "speed", fav.speed }, { "outfit", { { "type", fav.outfit.lookType }, { "head", fav.outfit.lookHead }, { "body", fav.outfit.lookBody }, { "legs", fav.outfit.lookLegs }, { "feet", fav.outfit.lookFeet }, { "addons", fav.outfit.lookAddon }, { "mount", fav.outfit.lookMount } } } });
	}
	j["favorites"] = favs;

	std::ofstream f(fn.GetFullPath().ToStdString());
	if (f.is_open()) {
		f << j.dump(4);
	}
}
