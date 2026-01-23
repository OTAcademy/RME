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
#include "rendering/ui/map_status_updater.h"
#include "gui.h"
#include "editor.h"
#include "map.h"
#include "tile.h"
#include "live_server.h"
#include "rendering/utilities/tile_describer.h"
#include "settings.h"

void MapStatusUpdater::Update(Editor& editor, int map_x, int map_y, int map_z) {
	wxString ss;
	ss << "x: " << map_x << " y:" << map_y << " z:" << map_z;
	g_gui.root->SetStatusText(ss, 2);

	ss = "";
	Tile* tile = editor.map.getTile(map_x, map_y, map_z);
	if (tile) {
		ss = TileDescriber::GetDescription(tile, g_settings.getInteger(Config::SHOW_SPAWNS), g_settings.getInteger(Config::SHOW_CREATURES));

		if (editor.IsLive()) {
			editor.GetLive().updateCursor(Position(map_x, map_y, map_z));
		}
		g_gui.root->SetStatusText(ss, 1);
	} else {
		g_gui.root->SetStatusText("Nothing", 1);
	}
}
