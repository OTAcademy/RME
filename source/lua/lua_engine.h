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

#ifndef RME_LUA_ENGINE_H
#define RME_LUA_ENGINE_H

#define SOL_ALL_SAFETIES_ON 1

#include <lua.hpp>
#include <sol/sol.hpp>

#include <string>
#include <functional>
#include <memory>

class LuaEngine {
public:
	LuaEngine();
	~LuaEngine();

	// Initialize the Lua state and register base APIs
	bool initialize();
	void shutdown();

	// Check if engine is ready
	bool isInitialized() const {
		return initialized;
	}

	// Script execution
	bool executeFile(const std::string& filepath);
	bool executeString(const std::string& code, const std::string& chunkName = "chunk");

	// Get underlying sol state for API registration
	sol::state& getState() {
		return lua;
	}
	const sol::state& getState() const {
		return lua;
	}

	// Error handling
	std::string getLastError() const {
		return lastError;
	}
	void clearError() {
		lastError.clear();
	}

	// Safe call wrapper with error handling
	template <typename Func>
	bool safeCall(Func&& func) {
		try {
			auto result = func();
			if (!result.valid()) {
				sol::error err = result;
				lastError = err.what();
				return false;
			}
			return true;
		} catch (const sol::error& e) {
			lastError = e.what();
			return false;
		} catch (const std::exception& e) {
			lastError = e.what();
			return false;
		}
	}

	// Safe call that doesn't return a result
	template <typename Func>
	bool safeCallVoid(Func&& func) {
		try {
			func();
			return true;
		} catch (const sol::error& e) {
			lastError = e.what();
			return false;
		} catch (const std::exception& e) {
			lastError = e.what();
			return false;
		}
	}

	// Print callback for console output
	using PrintCallback = std::function<void(const std::string&)>;
	void setPrintCallback(PrintCallback callback);

private:
	sol::state lua;
	std::string lastError;
	bool initialized;
	PrintCallback printCallback;

	void setupSandbox();
	void registerBaseLibraries();
};

#endif // RME_LUA_ENGINE_H
