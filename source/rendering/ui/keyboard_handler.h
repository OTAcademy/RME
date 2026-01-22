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

#ifndef RME_KEYBOARD_HANDLER_H_
#define RME_KEYBOARD_HANDLER_H_

class MapCanvas;
class wxKeyEvent;

/**
 * @brief Handles keyboard input for the MapCanvas.
 *
 * Extracts keyboard navigation, zoom, floor changes, and hotkey logic
 * from MapCanvas to improve separation of concerns.
 */
class KeyboardHandler {
public:
	/**
	 * @brief Handle key down event
	 * @param canvas The map canvas that received the event
	 * @param event The key event
	 */
	static void OnKeyDown(MapCanvas* canvas, wxKeyEvent& event);

	/**
	 * @brief Handle key up event
	 * @param canvas The map canvas that received the event
	 * @param event The key event
	 */
	static void OnKeyUp(MapCanvas* canvas, wxKeyEvent& event);

private:
	// Prevent instantiation
	KeyboardHandler() = delete;

	// Helper methods for specific key actions
	static void HandleFloorChange(MapCanvas* canvas, int keycode);
	static void HandleZoomKeys(MapCanvas* canvas, int keycode);
	static void HandleArrowNavigation(MapCanvas* canvas, wxKeyEvent& event);
	static void HandleBrushSizeChange(MapCanvas* canvas, int keycode);
	static void HandleHotkeys(MapCanvas* canvas, wxKeyEvent& event);
	static void HandleBrushVariation(MapCanvas* canvas, int keycode);
};

#endif // RME_KEYBOARD_HANDLER_H_
