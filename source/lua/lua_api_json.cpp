//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "lua_api_json.h"
#include "../json/json_spirit.h"

namespace LuaAPI {

	// Forward declarations
	sol::object valueToLua(const json_spirit::mValue& val, sol::state_view& lua);
	json_spirit::mValue luaToValue(const sol::object& obj);

	// Convert JSON Spirit Value to Lua Object
	sol::object valueToLua(const json_spirit::mValue& val, sol::state_view& lua) {
		switch (val.type()) {
			case json_spirit::null_type:
				return sol::nil;
			case json_spirit::bool_type:
				return sol::make_object(lua, val.get_bool());
			case json_spirit::int_type:
				return sol::make_object(lua, val.get_int());
			case json_spirit::real_type:
				return sol::make_object(lua, val.get_real());
			case json_spirit::str_type:
				return sol::make_object(lua, val.get_str());
			case json_spirit::array_type: {
				sol::table t = lua.create_table();
				const json_spirit::mArray& arr = val.get_array();
				for (size_t i = 0; i < arr.size(); ++i) {
					t[i + 1] = valueToLua(arr[i], lua); // Lua 1-based indexing
				}
				return t;
			}
			case json_spirit::obj_type: {
				sol::table t = lua.create_table();
				const json_spirit::mObject& obj = val.get_obj();
				for (const auto& pair : obj) {
					t[pair.first] = valueToLua(pair.second, lua);
				}
				return t;
			}
		}
		return sol::nil;
	}

	// Convert Lua Object to JSON Spirit Value
	json_spirit::mValue luaToValue(const sol::object& obj) {
		switch (obj.get_type()) {
			case sol::type::nil:
				return json_spirit::mValue(); // null
			case sol::type::boolean:
				return json_spirit::mValue(obj.as<bool>());
			case sol::type::number:
				// check if int or double, but Lua 5.1/JIT uses double mostly.
				// json_spirit distinguishes.
				// Let's just use double if floating point, int otherwise?
				// Simplification: use double for safety or check floor
				{
					double d = obj.as<double>();
					if (d == (int64_t)d) {
						return json_spirit::mValue((int64_t)d);
					}
					return json_spirit::mValue(d);
				}
			case sol::type::string:
				return json_spirit::mValue(obj.as<std::string>());
			case sol::type::table: {
				sol::table t = obj.as<sol::table>();
				// Determine if it's an array or object
				// Algorithm: allow explicit "is_array"? or check keys.
				// For now, check if it has key 1. If so, treat as array?
				// Or check length #t > 0.
				// Robust check: iterate keys. If all are integers 1..N, it's an array.

				bool isArray = true;
				size_t maxKey = 0;
				size_t count = 0;

				for (auto& pair : t) {
					count++;
					if (pair.first.get_type() == sol::type::number) {
						double k = pair.first.as<double>();
						if (k >= 1 && k == (size_t)k) {
							size_t idx = (size_t)k;
							if (idx > maxKey) {
								maxKey = idx;
							}
						} else {
							isArray = false;
							break;
						}
					} else {
						isArray = false;
						break;
					}
				}

				if (isArray && count > 0) {
					// We also check sparse arrays?
					// If maxKey == count, it's a dense array.
					if (maxKey != count) {
						// Sparse array or holes... treat as object with string keys?
						// Or just treat as sparse array? json_spirit doesn't support sparse arrays directly
						// standard JSON doesn't either (uses nulls).
						// Let's assume non-sparse for array.
						// If holes, better fail to object string keys.
						isArray = false;
					}
				}

				// Empty table {} -> Object by default in many JSON libs, or Array?
				// Lua {} is ambiguous. Let's default to Object (empty dict) as it's safer.
				if (count == 0) {
					isArray = false;
				}

				if (isArray) {
					json_spirit::mArray arr;
					arr.reserve(count);
					for (size_t i = 1; i <= count; ++i) {
						if (t[i].valid()) {
							arr.push_back(luaToValue(t[i]));
						} else {
							arr.push_back(json_spirit::mValue()); // null
						}
					}
					return json_spirit::mValue(arr);
				} else {
					json_spirit::mObject objVal;
					for (auto& pair : t) {
						std::string key;
						if (pair.first.get_type() == sol::type::string) {
							key = pair.first.as<std::string>();
						} else if (pair.first.get_type() == sol::type::number) {
							key = std::to_string(pair.first.as<int64_t>()); // Convert number keys to string for JSON object
						} else {
							continue; // Skip function keys etc
						}
						objVal[key] = luaToValue(pair.second);
					}
					return json_spirit::mValue(objVal);
				}
			}
			default:
				return json_spirit::mValue(); // null/ignore userdata/functions
		}
	}

	void registerJson(sol::state& lua) {
		// Create "json" table
		sol::table jsonTable = lua.create_table();

		jsonTable.set_function("encode", [](sol::object obj, sol::this_state s) -> std::string {
			json_spirit::mValue val = luaToValue(obj);
			return json_spirit::write(val); // minified output
		});

		jsonTable.set_function("encode_pretty", [](sol::object obj, sol::this_state s) -> std::string {
			json_spirit::mValue val = luaToValue(obj);
			return json_spirit::write_formatted(val); // pretty-printed output
		});

		jsonTable.set_function("decode", [](std::string jsonStr, sol::this_state s) -> sol::object {
			json_spirit::mValue val;
			if (json_spirit::read(jsonStr, val)) {
				sol::state_view lua(s);
				return valueToLua(val, lua);
			}
			return sol::nil;
		});

		lua["json"] = jsonTable;
	}

} // namespace LuaAPI
