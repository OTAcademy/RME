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
#include "rendering/ui/clipboard_handler.h"
#include "editor/editor.h"
#include "ui/gui.h"
#include "map/tile.h"
#include "game/item.h"
#include "editor/selection.h"
#include "app/settings.h"

void ClipboardHandler::copy(Editor& editor, int floor) {
	if (g_gui.IsSelectionMode()) {
		editor.copybuffer.copy(editor, floor);
	}
}

void ClipboardHandler::cut(Editor& editor, int floor) {
	if (g_gui.IsSelectionMode()) {
		editor.copybuffer.cut(editor, floor);
	}
	g_gui.RefreshView();
}

void ClipboardHandler::paste() {
	g_gui.DoPaste();
	g_gui.RefreshView();
}

void ClipboardHandler::doDelete(Editor& editor) {
	editor.destroySelection();
	g_gui.RefreshView();
}

void ClipboardHandler::copyPosition(const Selection& selection) {
	if (selection.empty()) {
		return;
	}

	Position minPos = selection.minPosition();
	Position maxPos = selection.maxPosition();

	std::ostringstream clip;
	if (minPos != maxPos) {
		clip << "{";
		clip << "fromx = " << minPos.x << ", ";
		clip << "tox = " << maxPos.x << ", ";
		clip << "fromy = " << minPos.y << ", ";
		clip << "toy = " << maxPos.y << ", ";
		if (minPos.z != maxPos.z) {
			clip << "fromz = " << minPos.z << ", ";
			clip << "toz = " << maxPos.z;
		} else {
			clip << "z = " << minPos.z;
		}
		clip << "}";
	} else {
		switch (g_settings.getInteger(Config::COPY_POSITION_FORMAT)) {
			case 0:
				clip << "{x = " << minPos.x << ", y = " << minPos.y << ", z = " << minPos.z << "}";
				break;
			case 1:
				clip << "{\"x\":" << minPos.x << ",\"y\":" << minPos.y << ",\"z\":" << minPos.z << "}";
				break;
			case 2:
				clip << minPos.x << ", " << minPos.y << ", " << minPos.z;
				break;
			case 3:
				clip << "(" << minPos.x << ", " << minPos.y << ", " << minPos.z << ")";
				break;
			case 4:
				clip << "Position(" << minPos.x << ", " << minPos.y << ", " << minPos.z << ")";
				break;
		}
	}

	if (wxTheClipboard->Open()) {
		wxTextDataObject* obj = new wxTextDataObject();
		obj->SetText(wxstr(clip.str()));
		wxTheClipboard->SetData(obj);

		wxTheClipboard->Close();
	}
}

void ClipboardHandler::copyServerId(const Selection& selection) {
	ASSERT(selection.size() == 1);

	if (wxTheClipboard->Open()) {
		Tile* tile = selection.getSelectedTile();
		ItemVector selected_items = tile->getSelectedItems();
		ASSERT(selected_items.size() == 1);

		const Item* item = selected_items.front();

		wxTextDataObject* obj = new wxTextDataObject();
		obj->SetText(i2ws(item->getID()));
		wxTheClipboard->SetData(obj);

		wxTheClipboard->Close();
	}
}

void ClipboardHandler::copyClientId(const Selection& selection) {
	ASSERT(selection.size() == 1);

	if (wxTheClipboard->Open()) {
		Tile* tile = selection.getSelectedTile();
		ItemVector selected_items = tile->getSelectedItems();
		ASSERT(selected_items.size() == 1);

		const Item* item = selected_items.front();

		wxTextDataObject* obj = new wxTextDataObject();
		obj->SetText(i2ws(item->getClientID()));
		wxTheClipboard->SetData(obj);

		wxTheClipboard->Close();
	}
}

void ClipboardHandler::copyName(const Selection& selection) {
	ASSERT(selection.size() == 1);

	if (wxTheClipboard->Open()) {
		Tile* tile = selection.getSelectedTile();
		ItemVector selected_items = tile->getSelectedItems();
		ASSERT(selected_items.size() == 1);

		const Item* item = selected_items.front();

		wxTextDataObject* obj = new wxTextDataObject();
		obj->SetText(wxstr(item->getName()));
		wxTheClipboard->SetData(obj);

		wxTheClipboard->Close();
	}
}
