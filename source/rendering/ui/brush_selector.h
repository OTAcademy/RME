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

#ifndef RME_BRUSH_SELECTOR_H_
#define RME_BRUSH_SELECTOR_H_

class Editor;
class Selection;
class Tile;

/**
 * @brief Handles brush selection from popup menu context.
 *
 * Extracts all OnSelect*Brush() methods from MapCanvas to improve
 * separation of concerns.
 */
class BrushSelector {
public:
	static void SelectRAWBrush(Selection& selection);
	static void SelectGroundBrush(Selection& selection);
	static void SelectDoodadBrush(Selection& selection);
	static void SelectDoorBrush(Selection& selection);
	static void SelectWallBrush(Selection& selection);
	static void SelectCarpetBrush(Selection& selection);
	static void SelectTableBrush(Selection& selection);
	static void SelectHouseBrush(Editor& editor, Selection& selection);
	static void SelectCollectionBrush(Selection& selection);
	static void SelectCreatureBrush(Selection& selection);
	static void SelectSpawnBrush();
	static void SelectSmartBrush(Editor& editor, Tile* tile);

private:
	BrushSelector() = delete;
};

#endif // RME_BRUSH_SELECTOR_H_
