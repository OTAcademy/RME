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

#ifndef RME_POPUP_ACTION_HANDLER_H_
#define RME_POPUP_ACTION_HANDLER_H_

class Editor;
class Tile;

/**
 * @brief Handles popup menu actions for the MapCanvas.
 *
 * Extracts tile-related actions from the popup menu that
 * require business logic separate from UI concerns.
 */
class PopupActionHandler {
public:
	static void RotateItem(Editor& editor);
	static void GotoDestination(Editor& editor);
	static void SwitchDoor(Editor& editor);
	static void BrowseTile(Editor& editor, int cursor_x, int cursor_y);
	static void OpenProperties(Editor& editor);
	static void SelectMoveTo(Editor& editor);

private:
	PopupActionHandler() = delete;
};

#endif // RME_POPUP_ACTION_HANDLER_H_
