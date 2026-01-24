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
#include "lua_api_selection.h"
#include "../selection.h"
#include "../tile.h"
#include "../position.h"
#include "../gui.h"
#include "../editor.h"

namespace LuaAPI {

	// Helper to get the current selection
	static Selection* getCurrentSelection() {
		Editor* editor = g_gui.GetCurrentEditor();
		if (!editor) {
			return nullptr;
		}
		return &editor->selection;
	}

	// Get tiles as a Lua table
	static sol::table getSelectionTiles(sol::this_state ts) {
		sol::state_view lua(ts);
		sol::table result = lua.create_table();

		Selection* sel = getCurrentSelection();
		if (!sel) {
			return result;
		}

		int idx = 1;
		for (Tile* tile : sel->getTiles()) {
			if (tile) {
				result[idx++] = tile;
			}
		}
		return result;
	}

	// Get bounds as a table with min/max positions
	static sol::table getSelectionBounds(sol::this_state ts) {
		sol::state_view lua(ts);
		sol::table result = lua.create_table();

		Selection* sel = getCurrentSelection();
		if (!sel || sel->size() == 0) {
			return result;
		}

		result["min"] = sel->minPosition();
		result["max"] = sel->maxPosition();
		return result;
	}

	void registerSelection(sol::state& lua) {
		// Register Selection usertype
		lua.new_usertype<Selection>(
			"Selection",
			// No public constructor - selection is obtained from app.selection
			sol::no_constructor,

			// Properties
			"isEmpty", sol::property([](Selection* sel) {
				return sel == nullptr || sel->size() == 0;
			}),
			"size", sol::property([](Selection* sel) -> size_t {
				return sel ? sel->size() : 0;
			}),
			"isBusy", sol::property([](Selection* sel) {
				return sel && sel->isBusy();
			}),

			// Tiles collection (as a table)
			"tiles", sol::property([](Selection* sel, sol::this_state ts) {
				return getSelectionTiles(ts);
			}),

			// Bounds
			"bounds", sol::property([](Selection* sel, sol::this_state ts) {
				return getSelectionBounds(ts);
			}),
			"minPosition", sol::property([](Selection* sel) -> Position {
				return sel ? sel->minPosition() : Position();
			}),
			"maxPosition", sol::property([](Selection* sel) -> Position {
				return sel ? sel->maxPosition() : Position();
			}),

			// Methods
			"start", [](Selection* sel) {
				if (sel && !sel->isBusy()) {
					sel->start(Selection::INTERNAL);
				} },
			"finish", [](Selection* sel) {
				if (sel && sel->isBusy()) {
					sel->finish(Selection::INTERNAL);
				} },
			"clear", [](Selection* sel) {
				if (sel) {
					bool managed = !sel->isBusy();
					if (managed){ sel->start(Selection::INTERNAL);
}
					sel->clear();
					if (managed){ sel->finish(Selection::INTERNAL);
}
				} },

			"add", sol::overload([](Selection* sel, Tile* tile) {
				if (sel && tile) {
					bool managed = !sel->isBusy();
					if (managed){ sel->start(Selection::INTERNAL);
}
					sel->add(tile);
					if (managed){ sel->finish(Selection::INTERNAL);
}
				} }, [](Selection* sel, Tile* tile, Item* item) {
				if (sel && tile && item) {
					bool managed = !sel->isBusy();
					if (managed){ sel->start(Selection::INTERNAL);
}
					sel->add(tile, item);
					if (managed){ sel->finish(Selection::INTERNAL);
}
				} }),

			"remove", sol::overload([](Selection* sel, Tile* tile) {
				if (sel && tile) {
					bool managed = !sel->isBusy();
					if (managed){ sel->start(Selection::INTERNAL);
}
					sel->remove(tile);
					if (managed){ sel->finish(Selection::INTERNAL);
}
				} }, [](Selection* sel, Tile* tile, Item* item) {
				if (sel && tile && item) {
					bool managed = !sel->isBusy();
					if (managed){ sel->start(Selection::INTERNAL);
}
					sel->remove(tile, item);
					if (managed){ sel->finish(Selection::INTERNAL);
}
				} }),

			// String representation
			sol::meta_function::to_string, [](Selection* sel) {
			if (!sel){ return std::string("Selection(invalid)");
}
			return "Selection(size=" + std::to_string(sel->size()) + ")"; }
		);
	}

} // namespace LuaAPI
