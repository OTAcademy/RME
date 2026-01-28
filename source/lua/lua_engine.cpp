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
#include "lua_engine.h"

#include <fstream>
#include <sstream>

LuaEngine::LuaEngine() :
	initialized(false) {
}

LuaEngine::~LuaEngine() {
	shutdown();
}

bool LuaEngine::initialize() {
	if (initialized) {
		return true;
	}

	try {
		// Open standard libraries
		lua.open_libraries(
			sol::lib::base,
			sol::lib::package,
			sol::lib::coroutine,
			sol::lib::string,
			sol::lib::table,
			sol::lib::math,
			sol::lib::utf8,
			sol::lib::io,
			sol::lib::os
		);

		// Setup sandbox (restrict dangerous functions)
		setupSandbox();

		// Register base libraries
		registerBaseLibraries();

		initialized = true;
		return true;
	} catch (const sol::error& e) {
		lastError = std::string("Failed to initialize Lua engine: ") + e.what();
		return false;
	} catch (const std::exception& e) {
		lastError = std::string("Failed to initialize Lua engine: ") + e.what();
		return false;
	}
}

void LuaEngine::shutdown() {
	if (!initialized) {
		return;
	}

	// Clear the Lua state
	lua = sol::state();
	initialized = false;
}

void LuaEngine::setupSandbox() {
	// Remove dangerous functions for security
	// We don't want scripts to be able to execute system commands or access arbitrary files

	// Sandbox OS library
	if (lua["os"].valid()) {
		lua["os"]["execute"] = sol::nil;
		lua["os"]["exit"] = sol::nil;
		lua["os"]["remove"] = sol::nil;
		lua["os"]["rename"] = sol::nil;
		lua["os"]["tmpname"] = sol::nil;
		lua["os"]["getenv"] = sol::nil;
		lua["os"]["setlocale"] = sol::nil;
	}

	// Disable IO library completely - scripts must use app.storage
	lua["io"] = sol::nil;

	// Disable dynamic loading of C libraries
	if (lua["package"].valid()) {
		lua["package"]["loadlib"] = sol::nil;

		// Also remove C loaders from package.searchers (Lua 5.2+) to prevent 'require' from loading DLLs
		if (lua["package"]["searchers"].valid()) {
			sol::table searchers = lua["package"]["searchers"];
			// 1: preload, 2: lua loader, 3: c loader, 4: all-in-one loader
			// Keep 1 and 2, remove 3 and 4
			searchers[3] = sol::nil;
			searchers[4] = sol::nil;
		}
	}

	// Custom safe dofile implementation
	lua["dofile"] = [this](const std::string& filename, sol::this_state s) -> bool {
		sol::state_view lua(s);

		// Get SCRIPT_DIR
		sol::object scriptDirObj = lua["SCRIPT_DIR"];
		if (!scriptDirObj.is<std::string>()) {
			throw sol::error("dofile: SCRIPT_DIR not set. Cannot resolve relative path.");
		}
		std::string scriptDir = scriptDirObj.as<std::string>();

		// Reject absolute paths
		if (filename.find(":") != std::string::npos || (filename.size() > 0 && (filename[0] == '/' || filename[0] == '\\'))) {
			throw sol::error("dofile: Absolute paths are not allowed. Use paths relative to the script.");
		}
		// Reject directory traversal
		if (filename.find("..") != std::string::npos) {
			throw sol::error("dofile: Directory traversal ('..') is not allowed.");
		}

		// Remove ./ prefix if present
		std::string cleanFilename = filename;
		if (cleanFilename.substr(0, 2) == "./" || cleanFilename.substr(0, 2) == ".\\") {
			cleanFilename = cleanFilename.substr(2);
		}

		std::string fullPath = scriptDir + "/" + cleanFilename;

		// We use executeFile which handles loading and error reporting
		return this->executeFile(fullPath);
	};

	// Secure 'load' to prevent bytecode execution (only allow mode "t")
	// If the chunk starts with the bytecode signature (ESC Lua), load() normally detects it.
	// By enforcing mode "t", we prevent loading precompiled bytecode.
	try {
		lua.script(R"(
			local old_load = load
			_G.load = function(chunk, chunkname, mode, env)
				-- If mode is provided, ensure it is 't'
				if mode and mode ~= "t" then
					error("Secure Mode: Binary chunks are disabled.")
				end
				-- Force 't' mode
				return old_load(chunk, chunkname, "t", env)
			end
		)");
	} catch (...) {
		// Ignore if load is already nil or something
	}
}

void LuaEngine::registerBaseLibraries() {
	// Register print function that outputs to the script console
	// Capture 'this' pointer for callback access
	lua["print"] = [this](sol::variadic_args va) {
		std::ostringstream oss;
		bool first = true;
		for (auto v : va) {
			if (!first) {
				oss << "\t";
			}
			first = false;

			// Convert value to string
			sol::state_view lua(v.lua_state());
			sol::function tostring = lua["tostring"];
			std::string str = tostring(v);
			oss << str;
		}

		std::string output = oss.str();

		// Output to callback if set, otherwise stdout
		if (printCallback) {
			printCallback(output);
		} else {
			std::cout << "[Lua] " << output << std::endl;
		}
	};

	// Register MouseButton enum
	lua.new_enum("MouseButton", "LEFT", 1, "RIGHT", 2, "MIDDLE", 3);
}

void LuaEngine::setPrintCallback(PrintCallback callback) {
	printCallback = callback;

	// Re-register print function with new callback
	lua["print"] = [this](sol::variadic_args va) {
		std::ostringstream oss;
		bool first = true;
		for (auto v : va) {
			if (!first) {
				oss << "\t";
			}
			first = false;

			sol::state_view lua(v.lua_state());
			sol::function tostring = lua["tostring"];
			std::string str = tostring(v);
			oss << str;
		}

		std::string output = oss.str();

		if (printCallback) {
			printCallback(output);
		} else {
			std::cout << "[Lua] " << output << std::endl;
		}
	};
}

bool LuaEngine::executeFile(const std::string& filepath) {
	if (!initialized) {
		lastError = "Lua engine not initialized";
		return false;
	}

	try {
		// Extract the directory from the filepath and set SCRIPT_DIR global
		std::string scriptDir;
		size_t lastSlash = filepath.find_last_of("/\\");
		if (lastSlash != std::string::npos) {
			scriptDir = filepath.substr(0, lastSlash);
		} else {
			scriptDir = ".";
		}
		lua["SCRIPT_DIR"] = scriptDir;

		sol::load_result loaded = lua.load_file(filepath);
		if (!loaded.valid()) {
			sol::error err = loaded;
			lastError = std::string("Failed to load script '") + filepath + "': " + err.what();
			return false;
		}

		sol::protected_function script = loaded;
		sol::protected_function_result result = script();

		if (!result.valid()) {
			sol::error err = result;
			lastError = std::string("Error executing script '") + filepath + "': " + err.what();
			return false;
		}

		return true;
	} catch (const sol::error& e) {
		lastError = std::string("Exception executing script '") + filepath + "': " + e.what();
		return false;
	} catch (const std::exception& e) {
		lastError = std::string("Exception executing script '") + filepath + "': " + e.what();
		return false;
	}
}

bool LuaEngine::executeString(const std::string& code, const std::string& chunkName) {
	if (!initialized) {
		lastError = "Lua engine not initialized";
		return false;
	}

	try {
		sol::load_result loaded = lua.load(code, chunkName);
		if (!loaded.valid()) {
			sol::error err = loaded;
			lastError = std::string("Failed to load code: ") + err.what();
			return false;
		}

		sol::protected_function script = loaded;
		sol::protected_function_result result = script();

		if (!result.valid()) {
			sol::error err = result;
			lastError = std::string("Error executing code: ") + err.what();
			return false;
		}

		return true;
	} catch (const sol::error& e) {
		lastError = std::string("Exception executing code: ") + e.what();
		return false;
	} catch (const std::exception& e) {
		lastError = std::string("Exception executing code: ") + e.what();
		return false;
	}
}
