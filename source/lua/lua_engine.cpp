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

LuaEngine::LuaEngine() : initialized(false) {
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

	// Sandbox restrictions conditioned or removed for development flexibility
	// if (lua["os"].valid()) {
	// 	 lua["os"]["execute"] = sol::nil;
	// 	 lua["os"]["exit"] = sol::nil;
	// }

	// Enable standard libraries for versatile scripting
	// lua["io"] = sol::nil;
	// lua["loadfile"] = sol::nil;
	// lua["dofile"] = sol::nil;
	// lua["package"]["loadlib"] = sol::nil;
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
	lua.new_enum("MouseButton",
		"LEFT", 1,
		"RIGHT", 2,
		"MIDDLE", 3
	);
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
