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

#include "app/main.h"

#include "editor/selection_thread.h"
#include "editor/editor.h"
#include "map/map.h"
#include "app/settings.h"
#include "editor/action.h"

SelectionThread::SelectionThread(Editor& editor, Position start, Position end) :
	wxThread(wxTHREAD_JOINABLE),
	editor(editor),
	start(start),
	end(end),
	selection(editor),
	result(nullptr) {
	////
}

SelectionThread::~SelectionThread() {
	////
}

void SelectionThread::Execute() {
	Create();
	Run();
}

wxThread::ExitCode SelectionThread::Entry() {
	selection.start(Selection::SUBTHREAD);
	for (int z = start.z; z >= end.z; --z) {
		for (int x = start.x; x <= end.x; ++x) {
			for (int y = start.y; y <= end.y; ++y) {
				Tile* tile = editor.map.getTile(x, y, z);
				if (!tile) {
					continue;
				}

				selection.add(tile);
			}
		}
		if (z <= GROUND_LAYER && g_settings.getInteger(Config::COMPENSATED_SELECT)) {
			++start.x;
			++start.y;
			++end.x;
			++end.y;
		}
	}
	// Access wrapper to get subsession
	// Since SelectionThread is friend of Selection, we can access private members of selection instance
	result = selection.subsession;
	selection.finish(Selection::SUBTHREAD);

	return nullptr;
}
