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

#include "main.h"
#include "lua_api_app.h"
#include "lua_script_manager.h"
#include "../gui.h"
#include "../editor.h"
#include "../map.h"
#include "../brush.h"
#include "../action.h"
#include "../tile.h"
#include "../selection.h"
#include "../items.h"
#include "../raw_brush.h"

#include <wx/msgdlg.h>
#include <wx/app.h>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <unordered_map>
#include <cstdio>
#include <thread>
#include <chrono>

namespace LuaAPI {

// ============================================================================
// Transaction System for Undo/Redo Support
// ============================================================================

// Transaction implementation
#include "../selection.h"
#include "../house.h"

	// Helper to sync map metadata when swapping tiles
	static void updateTileMetadata(Editor* editor, Tile* tile, bool adding) {
		if (!editor || !tile) {
			return;
		}
		Map* map = editor->getMap();
		if (!map) {
			return;
		}

		if (adding) {
			if (tile->spawn) {
				map->addSpawn(tile);
			}
			if (tile->getHouseID()) {
				House* h = map->houses.getHouse(tile->getHouseID());
				if (h) {
					h->addTile(tile);
				}
			}
		} else {
			if (tile->spawn) {
				map->removeSpawn(tile);
			}
			if (tile->getHouseID()) {
				House* h = map->houses.getHouse(tile->getHouseID());
				if (h) {
					h->removeTile(tile);
				}
			}
			// Also clean up selection if removing
			if (tile->isSelected()) {
				// We use internal session to avoid creating undo actions for this cleanup
				editor->selection.start(Selection::INTERNAL);
				editor->selection.removeInternal(tile);
				editor->selection.finish(Selection::INTERNAL);
			}
		}
	}

	class LuaTransaction {
		bool active;
		Editor* editor;
		BatchAction* batch;
		Action* action;
		std::unordered_map<uint64_t, Tile*> originalTiles;

		uint64_t positionKey(const Position& pos) const {
			return (static_cast<uint64_t>(pos.x) << 32) | (static_cast<uint64_t>(pos.y) << 16) | static_cast<uint64_t>(pos.z);
		}

	public:
		static LuaTransaction& getInstance() {
			static LuaTransaction instance;
			return instance;
		}

		LuaTransaction() :
			active(false), editor(nullptr), batch(nullptr), action(nullptr) { }

		void begin(Editor* ed) {
			if (active) {
				throw sol::error("Transaction already in progress");
			}

			editor = ed;
			if (!editor || !editor->actionQueue) {
				throw sol::error("No editor or action queue available");
			}

			active = true;
			batch = editor->actionQueue->createBatch(ACTION_LUA_SCRIPT);
			action = editor->actionQueue->createAction(ACTION_LUA_SCRIPT);
			originalTiles.clear();
		}

		void commit() {
			if (!active) {
				return;
			}

			// Process each modified tile
			for (auto& pair : originalTiles) {
				Tile* originalTile = pair.second;
				Position pos = originalTile->getPosition();

				// Get the current (modified) tile from the map
				Tile* modifiedTile = editor->getMap()->getTile(pos);
				if (modifiedTile) {
					// Create a deep copy of the modified tile - this is what we want as the "new" state
					Tile* modifiedCopy = modifiedTile->deepCopy(*editor->getMap());

					// Swap the original back into the map
					Tile* swappedOut = editor->getMap()->swapTile(pos, originalTile);

					// swappedOut should be the modifiedTile. We need to clean it up.
					// Remove it from Map metadata (spawns, houses) and selection
					updateTileMetadata(editor, swappedOut, false);

					// Add originalTile back to Map metadata
					updateTileMetadata(editor, originalTile, true);

					delete swappedOut;

					// Create Change with the modified copy
					// When actions commit, they will swap modifiedCopy in and originalTile out.
					// The Action system handles metadata updates during its commit/undo.
					Change* change = new Change(modifiedCopy);
					action->addChange(change);
				} else {
					// No real change or tile was removed, cleanup
					delete originalTile;
				}
			}

			// Clear - ownership has been transferred
			originalTiles.clear();

			if (action->size() > 0) {
				batch->addAndCommitAction(action);
				editor->addBatch(batch);
				editor->getMap()->doChange();
				g_gui.RefreshView(); // Force redraw immediately
			} else {
				// No changes, clean up
				delete action;
				delete batch;
			}

			cleanup();
		}

		void rollback() {
			if (!active) {
				return;
			}

			// Restore original tiles (discard any changes made)
			for (auto& pair : originalTiles) {
				Tile* originalTile = pair.second;
				if (originalTile) {
					Position pos = originalTile->getPosition();
					Tile* modifiedTile = editor->getMap()->swapTile(pos, originalTile);

					// Clean up modified tile
					updateTileMetadata(editor, modifiedTile, false);

					// Restore original tile metadata
					updateTileMetadata(editor, originalTile, true);

					delete modifiedTile; // Discard the modified version
				}
			}
			originalTiles.clear();

			// Discard without committing
			delete action;
			delete batch;

			cleanup();
		}

		void markTileModified(Tile* tile) {
			if (!active || !tile) {
				return;
			}

			Position pos = tile->getPosition();
			uint64_t key = positionKey(pos);

			// Only snapshot the tile once per transaction (first time it's modified)
			if (originalTiles.find(key) == originalTiles.end()) {
				// Create a deep copy of the ORIGINAL tile BEFORE modification
				Tile* originalCopy = tile->deepCopy(*editor->getMap());
				originalTiles[key] = originalCopy;
			}
		}

		bool isActive() const {
			return active;
		}
		Editor* getEditor() const {
			return editor;
		}

	private:
		void cleanup() {
			active = false;
			editor = nullptr;
			batch = nullptr;
			action = nullptr;
			// Don't clear originalTiles here - it should be empty or ownership transferred
		}
	};

	// Global accessor for tile modification tracking (used by lua_api_tile.cpp)
	void markTileForUndo(Tile* tile) {
		if (LuaTransaction::getInstance().isActive()) {
			LuaTransaction::getInstance().markTileModified(tile);
		}
	}

	// ============================================================================
	// Helper Functions
	// ============================================================================

	// Helper function to show alert dialog
	static int showAlert(sol::this_state ts, sol::object arg) {
		sol::state_view lua(ts);

		std::string title = "Script";
		std::string message;
		std::vector<std::string> buttons;

		// Handle different argument types
		if (arg.is<std::string>()) {
			message = arg.as<std::string>();
			buttons.push_back("OK");
		} else if (arg.is<sol::table>()) {
			sol::table opts = arg.as<sol::table>();

			if (opts["title"].valid()) {
				title = opts["title"].get<std::string>();
			}
			if (opts["text"].valid()) {
				message = opts["text"].get<std::string>();
			}
			if (opts["buttons"].valid()) {
				sol::table btns = opts["buttons"];
				for (size_t i = 1; i <= btns.size(); ++i) {
					if (btns[i].valid()) {
						buttons.push_back(btns[i].get<std::string>());
					}
				}
			}

			if (buttons.empty()) {
				buttons.push_back("OK");
			}
		} else {
			sol::function tostring = lua["tostring"];
			message = tostring(arg);
			buttons.push_back("OK");
		}

		// Determine dialog style based on buttons
		long style = wxCENTRE;
		if (buttons.size() == 1) {
			style |= wxOK;
		} else if (buttons.size() == 2) {
			std::string btn1 = buttons[0];
			std::string btn2 = buttons[1];
			std::transform(btn1.begin(), btn1.end(), btn1.begin(), ::tolower);
			std::transform(btn2.begin(), btn2.end(), btn2.begin(), ::tolower);

			if ((btn1 == "ok" && btn2 == "cancel") || (btn1 == "cancel" && btn2 == "ok")) {
				style |= wxOK | wxCANCEL;
			} else if ((btn1 == "yes" && btn2 == "no") || (btn1 == "no" && btn2 == "yes")) {
				style |= wxYES_NO;
			} else {
				style |= wxOK | wxCANCEL;
			}
		} else if (buttons.size() >= 3) {
			style |= wxYES_NO | wxCANCEL;
		}

		wxWindow* parent = g_gui.root;
		wxMessageDialog dlg(parent, wxString(message), wxString(title), style);

		int result = dlg.ShowModal();

		switch (result) {
			case wxID_OK:
			case wxID_YES:
				return 1;
			case wxID_NO:
				return 2;
			case wxID_CANCEL:
				return buttons.size() >= 3 ? 3 : 2;
			default:
				return 0;
		}
	}

	// Check if a map is currently open
	static bool hasMap() {
		Editor* editor = g_gui.GetCurrentEditor();
		return editor != nullptr && editor->getMap() != nullptr;
	}

	// Refresh the map view
	static void refresh() {
		g_gui.RefreshView();
	}

	// Get the current Map object
	static Map* getMap() {
		Editor* editor = g_gui.GetCurrentEditor();
		if (!editor) {
			return nullptr;
		}
		return editor->getMap();
	}

	// Get the current Selection object
	static Selection* getSelection() {
		Editor* editor = g_gui.GetCurrentEditor();
		if (!editor) {
			return nullptr;
		}
		return &editor->selection;
	}

	static void setClipboard(const std::string& text) {
		if (wxTheClipboard->Open()) {
			wxTheClipboard->SetData(new wxTextDataObject(text));
			wxTheClipboard->Close();
		}
	}

	// Transaction function with undo/redo support
	static void transaction(const std::string& name, sol::function func) {
		Editor* editor = g_gui.GetCurrentEditor();
		if (!editor) {
			throw sol::error("No map open");
		}

		LuaTransaction& trans = LuaTransaction::getInstance();

		try {
			trans.begin(editor);
			func();
			trans.commit();
			g_gui.RefreshView();
		} catch (const sol::error&) {
			trans.rollback();
			throw; // Re-throw to show error to user
		} catch (const std::exception& e) {
			trans.rollback();
			throw sol::error(std::string("Transaction failed: ") + e.what());
		} catch (...) {
			trans.rollback();
			throw sol::error("Transaction failed with unknown error");
		}
	}

	static sol::object getBorders(sol::this_state ts) {
		sol::state_view lua(ts);

		sol::table bordersTable = lua.create_table();

		for (auto& pair : g_brushes.getBorders()) {
			AutoBorder* border = pair.second;
			if (!border) {
				continue;
			}

			sol::table b = lua.create_table();
			b["id"] = border->id;
			b["group"] = border->group;
			b["ground"] = border->ground;

			sol::table tiles = lua.create_table();
			for (int i = 0; i < 13; ++i) {
				tiles[i + 1] = border->tiles[i];
			}
			b["tiles"] = tiles;

			bordersTable[border->id] = b;
		}

		return bordersTable;
	}

	static std::string getDataDirectory() {
		return GUI::GetDataDirectory().ToStdString();
	}

	static sol::table storageForScript(sol::this_state ts, const std::string& name) {
		sol::state_view lua(ts);
		sol::table storage = lua.create_table();

		std::string scriptDir = ".";
		if (lua["SCRIPT_DIR"].valid()) {
			scriptDir = lua["SCRIPT_DIR"];
		}

		std::string filename = name;

		// Security check: Prevent path traversal and absolute paths
		if (filename.find("..") != std::string::npos || std::filesystem::path(filename).is_absolute()) {
			printf("[Lua Security] Blocked unsafe path in app.storage: %s\n", filename.c_str());
			return lua.create_table();
		}

		if (filename.find('.') == std::string::npos) {
			filename += ".json";
		}
		std::string path = scriptDir + "/" + filename;
		storage["path"] = path;

		storage["load"] = [path](sol::this_state ts2, sol::object) -> sol::object {
			sol::state_view lua(ts2);
			std::ifstream file(path);
			if (!file.is_open()) {
				return sol::make_object(lua, sol::nil);
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();

			std::string content = buffer.str();
			if (content.empty()) {
				return sol::make_object(lua, sol::nil);
			}

			sol::table json = lua["json"];
			if (!json.valid() || !json["decode"].valid()) {
				return sol::make_object(lua, sol::nil);
			}

			try {
				sol::function decode = json["decode"];
				sol::protected_function_result result = decode(content);
				if (!result.valid()) {
					return sol::make_object(lua, sol::nil);
				}
				sol::object decoded = result;
				return sol::make_object(lua, decoded);
			} catch (const sol::error&) {
				return sol::make_object(lua, sol::nil);
			}
		};

		storage["save"] = [path](sol::this_state ts2, sol::object first, sol::object second) -> bool {
			sol::state_view lua(ts2);
			std::string content;

			sol::object data = (second.valid() && !second.is<sol::nil_t>()) ? second : first;
			if (!data.valid() || data.is<sol::nil_t>()) {
				return false;
			}

			if (data.is<std::string>()) {
				content = data.as<std::string>();
			} else {
				sol::table json = lua["json"];
				if (!json.valid() || !json["encode_pretty"].valid()) {
					return false;
				}
				try {
					sol::function encode = json["encode_pretty"];
					sol::protected_function_result result = encode(data);
					if (!result.valid()) {
						return false;
					}
					content = result.get<std::string>();
				} catch (const sol::error&) {
					return false;
				}
			}

			std::ofstream file(path, std::ios::trunc);
			if (!file.is_open()) {
				return false;
			}
			file << content;
			file.close();
			return true;
		};

		storage["clear"] = [path](sol::object) -> bool {
			return std::remove(path.c_str()) == 0;
		};

		return storage;
	}

	// ============================================================================
	// Register App API
	// ============================================================================

	void registerApp(sol::state& lua) {
		// Create the 'app' table
		sol::table app = lua.create_named_table("app");

		// Version info
		app["version"] = __RME_VERSION__;
		app["apiVersion"] = 1;

		// Functions
		app["alert"] = showAlert;
		app["hasMap"] = hasMap;
		app["refresh"] = refresh;
		app["transaction"] = transaction;
		app["setClipboard"] = setClipboard;
		app["getDataDirectory"] = getDataDirectory;
		app["addContextMenu"] = [](const std::string& label, sol::function callback) {
			g_luaScripts.registerContextMenuItem(label, callback);
		};
		app["selectRaw"] = [](int itemId) {
			if (g_items.typeExists(itemId)) {
				ItemType& it = g_items[itemId];
				if (it.raw_brush) {
					g_gui.SelectBrush(it.raw_brush, TILESET_RAW);
				}
			}
		};

		app["setCameraPosition"] = [](int x, int y, int z) {
			g_gui.SetScreenCenterPosition(Position(x, y, z));
		};
		app["storage"] = storageForScript;

		// Yield to process pending UI events (prevents UI freeze during long operations)
		app["yield"] = []() {
			if (wxTheApp) {
				wxTheApp->Yield(true);
			}
		};

		// Sleep for a given number of milliseconds (use sparingly, blocks the UI)
		app["sleep"] = [](int milliseconds) {
			if (milliseconds > 0 && milliseconds <= 10000) { // Max 10 seconds
				std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
			}
		};

		// Get elapsed time in milliseconds since application start (high precision timer)
		app["getTime"] = []() -> long {
			return g_gui.gfx.getElapsedTime();
		};

		// Event system: app.events:on("eventName", callback) / app.events:off(id)
		sol::table events = lua.create_table();
		events["on"] = [](sol::this_state ts, sol::table self, const std::string& eventName, sol::function callback) -> int {
			return g_luaScripts.addEventListener(eventName, callback);
		};
		events["off"] = [](sol::this_state ts, sol::table self, int listenerId) -> bool {
			return g_luaScripts.removeEventListener(listenerId);
		};
		app["events"] = events;

		// Properties via metatable (for dynamic properties like 'map' and 'selection')
		sol::table mt = lua.create_table();
		mt[sol::meta_function::index] = [](sol::this_state ts, sol::table self, std::string key) -> sol::object {
			sol::state_view lua(ts);

			if (key == "map") {
				Map* map = getMap();
				if (map) {
					return sol::make_object(lua, map);
				}
				return sol::nil;
			} else if (key == "selection") {
				Selection* sel = getSelection();
				if (sel) {
					return sol::make_object(lua, sel);
				}
				return sol::nil;
			} else if (key == "borders") {
				return getBorders(ts);
			} else if (key == "editor") {
				Editor* editor = g_gui.GetCurrentEditor();
				if (editor) {
					return sol::make_object(lua, editor);
				}
				return sol::nil;
			} else if (key == "brush") {
				Brush* b = g_gui.GetCurrentBrush();
				if (b) {
					return sol::make_object(lua, b);
				}
				return sol::nil;
			} else if (key == "brushSize") {
				return sol::make_object(lua, g_gui.GetBrushSize());
			} else if (key == "brushShape") {
				return sol::make_object(lua, g_gui.GetBrushShape() == BRUSHSHAPE_CIRCLE ? "circle" : "square");
			} else if (key == "brushVariation") {
				return sol::make_object(lua, g_gui.GetBrushVariation());
			} else if (key == "spawnTime") {
				return sol::make_object(lua, g_gui.GetSpawnTime());
			}
			return sol::nil;
		};

		mt[sol::meta_function::new_index] = [](sol::this_state ts, sol::table self, std::string key, sol::object value) {
			if (key == "brushSize") {
				if (value.is<int>()) {
					g_gui.SetBrushSize(value.as<int>());
				}
			} else if (key == "brushShape") {
				if (value.is<std::string>()) {
					std::string s = value.as<std::string>();
					if (s == "circle") {
						g_gui.SetBrushShape(BRUSHSHAPE_CIRCLE);
					} else if (s == "square") {
						g_gui.SetBrushShape(BRUSHSHAPE_SQUARE);
					}
				}
			} else if (key == "brushVariation") {
				if (value.is<int>()) {
					g_gui.SetBrushVariation(value.as<int>());
				}
			} else if (key == "spawnTime") {
				if (value.is<int>()) {
					g_gui.SetSpawnTime(value.as<int>());
				}
			}
		};

		// Map overlay system
		sol::table mapView = lua.create_table();
		mapView["addOverlay"] = [](sol::variadic_args va) -> bool {
			if (va.size() == 2 && va[0].is<std::string>() && va[1].is<sol::table>()) {
				return g_luaScripts.addMapOverlay(va[0].as<std::string>(), va[1].as<sol::table>());
			}
			if (va.size() == 3 && va[1].is<std::string>() && va[2].is<sol::table>()) {
				return g_luaScripts.addMapOverlay(va[1].as<std::string>(), va[2].as<sol::table>());
			}
			return false;
		};
		mapView["removeOverlay"] = [](sol::variadic_args va) -> bool {
			if (va.size() == 1 && va[0].is<std::string>()) {
				return g_luaScripts.removeMapOverlay(va[0].as<std::string>());
			}
			if (va.size() == 2 && va[1].is<std::string>()) {
				return g_luaScripts.removeMapOverlay(va[1].as<std::string>());
			}
			return false;
		};
		mapView["setEnabled"] = [](sol::variadic_args va) -> bool {
			if (va.size() == 2 && va[0].is<std::string>() && va[1].is<bool>()) {
				return g_luaScripts.setMapOverlayEnabled(va[0].as<std::string>(), va[1].as<bool>());
			}
			if (va.size() == 3 && va[1].is<std::string>() && va[2].is<bool>()) {
				return g_luaScripts.setMapOverlayEnabled(va[1].as<std::string>(), va[2].as<bool>());
			}
			return false;
		};
		mapView["registerShow"] = [](sol::variadic_args va) -> bool {
			std::string label;
			std::string overlayId;
			bool enabled = true;
			sol::function ontoggle;

			if (va.size() == 2 && va[0].is<std::string>() && va[1].is<std::string>()) {
				label = va[0].as<std::string>();
				overlayId = va[1].as<std::string>();
			} else if (va.size() >= 3) {
				if (va[0].is<sol::table>()) {
					if (va[1].is<std::string>()) {
						label = va[1].as<std::string>();
					}
					if (va[2].is<std::string>()) {
						overlayId = va[2].as<std::string>();
					}
				} else if (va[0].is<std::string>() && va[1].is<std::string>()) {
					label = va[0].as<std::string>();
					overlayId = va[1].as<std::string>();
				}
			}

			if (va.size() >= 3 && va[va.size() - 1].is<sol::table>()) {
				sol::table opts = va[va.size() - 1].as<sol::table>();
				enabled = opts.get_or(std::string("enabled"), enabled);
				if (opts["ontoggle"].valid()) {
					ontoggle = opts["ontoggle"];
				}
			} else if (va.size() >= 3 && va[va.size() - 1].is<bool>()) {
				enabled = va[va.size() - 1].as<bool>();
			}

			if (label.empty() || overlayId.empty()) {
				return false;
			}

			return g_luaScripts.registerMapOverlayShow(label, overlayId, enabled, ontoggle);
		};
		app["mapView"] = mapView;

		app[sol::metatable_key] = mt;

		// Register Editor usertype for undo/redo functionality
		lua.new_usertype<Editor>(
			"Editor",
			sol::no_constructor,

			// Undo/Redo functions
			"undo", [](Editor* editor) {
			if (editor && editor->actionQueue && editor->actionQueue->canUndo()) {
				editor->actionQueue->undo();
				g_gui.RefreshView();
			} },
			"redo", [](Editor* editor) {
			if (editor && editor->actionQueue && editor->actionQueue->canRedo()) {
				editor->actionQueue->redo();
				g_gui.RefreshView();
			} },
			"canUndo", [](Editor* editor) -> bool { return editor && editor->actionQueue && editor->actionQueue->canUndo(); },
			"canRedo", [](Editor* editor) -> bool { return editor && editor->actionQueue && editor->actionQueue->canRedo(); },

			// History info
			"historyIndex", sol::property([](Editor* editor) -> int {
				if (editor && editor->actionQueue) {
					return (int)editor->actionQueue->getCurrentIndex();
				}
				return 0;
			}),
			"historySize", sol::property([](Editor* editor) -> int {
				if (editor && editor->actionQueue) {
					return (int)editor->actionQueue->getSize();
				}
				return 0;
			}),

			// Get history as a table
			"getHistory", [](Editor* editor, sol::this_state ts) -> sol::table {
			sol::state_view lua(ts);
			sol::table history = lua.create_table();

			if (editor && editor->actionQueue) {
				size_t size = editor->actionQueue->getSize();
				for (size_t i = 0; i < size; ++i) {
					sol::table input = lua.create_table();
					input["index"] = (int)(i + 1); // 1-based for Lua
					input["name"] = editor->actionQueue->getActionName(i);
					history[i + 1] = input;
				}
			}
			return history; },

			// Navigate to specific history index
			"goToHistory", [](Editor* editor, int targetIndex) {
			if (!editor || !editor->actionQueue){ return;
}

			int current = (int)editor->actionQueue->getCurrentIndex();
			int target = targetIndex; // Already 1-based from Lua

			if (target < 0){ target = 0;
}
			if (target > (int)editor->actionQueue->getSize()) {
				target = (int)editor->actionQueue->getSize();
			}

			int diff = target - current;

			if (diff > 0) {
				for (int i = 0; i < diff; ++i) {
					if (editor->actionQueue->canRedo()) {
						editor->actionQueue->redo();
					}
				}
			} else if (diff < 0) {
				for (int i = 0; i < -diff; ++i) {
					if (editor->actionQueue->canUndo()) {
						editor->actionQueue->undo();
					}
				}
			}
			g_gui.RefreshView(); }
		);
	}

} // namespace LuaAPI
