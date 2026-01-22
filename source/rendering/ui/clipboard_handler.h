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

#ifndef RME_CLIPBOARD_HANDLER_H_
#define RME_CLIPBOARD_HANDLER_H_

class Editor;
class Selection;

// Static helper class for clipboard operations
class ClipboardHandler {
public:
	// Map tile/item clipboard operations
	static void copy(Editor& editor, int floor);
	static void cut(Editor& editor, int floor);
	static void paste();
	static void doDelete(Editor& editor);

	// Text clipboard operations
	static void copyPosition(const Selection& selection);
	static void copyServerId(const Selection& selection);
	static void copyClientId(const Selection& selection);
	static void copyName(const Selection& selection);
};

#endif
