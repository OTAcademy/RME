//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_LUA_SCRIPT_MANAGER_H
#define RME_LUA_SCRIPT_MANAGER_H

#include "lua_engine.h"
#include "lua_script.h"

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

#include "../map_overlay.h"

class Tile;
class Item;

// Callback type for output messages
using LuaOutputCallback = std::function<void(const std::string& message, bool isError)>;

class LuaScriptManager {
public:
	static LuaScriptManager& getInstance();

	// Lifecycle
	bool initialize();
	void shutdown();
	bool isInitialized() const {
		return initialized;
	}

	// Script discovery and management
	void discoverScripts();
	void reloadScripts();

	// Script execution
	bool executeScript(const std::string& filepath);
	bool executeScript(LuaScript* script);
	bool executeScript(size_t index, std::string& errorOut);

	// Access scripts
	const std::vector<std::unique_ptr<LuaScript>>& getScripts() const {
		return scripts;
	}
	LuaScript* getScript(const std::string& filepath);

	// Enable/disable scripts
	void setScriptEnabled(size_t index, bool enabled);
	bool isScriptEnabled(size_t index) const;

	// Engine access
	LuaEngine& getEngine() {
		return engine;
	}

	// Error handling
	std::string getLastError() const {
		return lastError;
	}

	// Scripts directory
	std::string getScriptsDirectory() const;
	void openScriptsFolder();

	// Output callback for Script Manager window
	void setOutputCallback(LuaOutputCallback callback);
	void logOutput(const std::string& message, bool isError = false);

	// Dynamic Context Menu
	struct ContextMenuItem {
		std::string label;
		sol::function callback;
	};
	void registerContextMenuItem(const std::string& label, sol::function callback);
	const std::vector<ContextMenuItem>& getContextMenuItems() const {
		return contextMenuItems;
	}

	// Event System
	struct EventListener {
		int id;
		std::string eventName;
		sol::function callback;
	};
	int addEventListener(const std::string& eventName, sol::function callback);
	bool removeEventListener(int listenerId);

	template <typename... Args>
	void emit(const std::string& eventName, Args&&... args) {
		if (!initialized) {
			return;
		}

		// Iterate over a copy to allow callbacks to modify the listener list safely
		std::vector<EventListener> listenersCopy = eventListeners;
		for (const auto& listener : listenersCopy) {
			if (listener.eventName == eventName && listener.callback.valid()) {
				try {
					listener.callback(std::forward<Args>(args)...);
				} catch (const sol::error& e) {
					logOutput("Event '" + eventName + "' error: " + std::string(e.what()), true);
				}
			}
		}
	}

	template <typename... Args>
	bool emitCancellable(const std::string& eventName, Args&&... args) {
		if (!initialized) {
			return false;
		}

		// Iterate over a copy to allow callbacks to modify the listener list safely
		std::vector<EventListener> listenersCopy = eventListeners;
		bool consumed = false;
		for (const auto& listener : listenersCopy) {
			if (listener.eventName == eventName && listener.callback.valid()) {
				try {
					sol::object result = listener.callback(std::forward<Args>(args)...);
					if (result.valid() && result.is<bool>() && result.as<bool>()) {
						consumed = true;
						break; // Stop propagation
					}
				} catch (const sol::error& e) {
					logOutput("Event '" + eventName + "' error: " + std::string(e.what()), true);
				}
			}
		}
		return consumed;
	}

	// Clear all registered callbacks (called before script reload)
	void clearAllCallbacks();

	// Map Overlay System
	struct MapOverlay {
		std::string id;
		bool enabled = true;
		int order = 0;
		sol::function ondraw;
		sol::function onhover;
	};
	struct MapOverlayShowItem {
		std::string label;
		std::string overlayId;
		bool enabled = true;
		sol::function ontoggle;
	};
	bool addMapOverlay(const std::string& id, sol::table options);
	bool removeMapOverlay(const std::string& id);
	bool setMapOverlayEnabled(const std::string& id, bool enabled);
	bool registerMapOverlayShow(const std::string& label, const std::string& overlayId, bool enabled, sol::function ontoggle);
	bool setMapOverlayShowEnabled(const std::string& overlayId, bool enabled);
	bool isMapOverlayEnabled(const std::string& id) const;
	const std::vector<MapOverlayShowItem>& getMapOverlayShows() const {
		return mapOverlayShows;
	}
	void collectMapOverlayCommands(const MapViewInfo& view, std::vector<MapOverlayCommand>& out);
	void updateMapOverlayHover(int map_x, int map_y, int map_z, int screen_x, int screen_y, Tile* tile, Item* topItem);
	const MapOverlayHoverState& getMapOverlayHover() const {
		return mapOverlayHover;
	}

private:
	LuaScriptManager() = default;
	~LuaScriptManager() = default;
	LuaScriptManager(const LuaScriptManager&) = delete;
	LuaScriptManager& operator=(const LuaScriptManager&) = delete;

	LuaEngine engine;
	std::vector<std::unique_ptr<LuaScript>> scripts;
	std::string lastError;
	bool initialized = false;
	LuaOutputCallback outputCallback;
	std::vector<ContextMenuItem> contextMenuItems;
	std::vector<EventListener> eventListeners;
	int nextListenerId = 1;
	std::vector<MapOverlay> mapOverlays;
	std::vector<MapOverlayShowItem> mapOverlayShows;
	MapOverlayHoverState mapOverlayHover;

	void registerAPIs();
	void scanDirectory(const std::string& directory);
	void runAutoScripts();
};

// Global accessor macro
#define g_luaScripts LuaScriptManager::getInstance()

#endif // RME_LUA_SCRIPT_MANAGER_H
