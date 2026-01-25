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

#ifndef RME_MAP_WINDOWS_H_
#define RME_MAP_WINDOWS_H_

#include "app/main.h"

#include "ui/dcbutton.h"
#include "ui/positionctrl.h"

#include "ui/map/map_properties_window.h"
#include "ui/map/towns_window.h"
#include "ui/dialogs/find_dialog.h"
#include "ui/map/import_map_window.h"
#include "ui/map/export_minimap_window.h"
#include "ui/map/export_tilesets_window.h"

class GameSprite;
class MapTab;

/**
 * A toggle button with an item on it.
 */
class ItemToggleButton : public DCButton {
public:
	ItemToggleButton(wxWindow* parent, RenderSize size, int lookid, wxWindowID id = wxID_ANY) :
		DCButton(parent, id, wxDefaultPosition, DC_BTN_TOGGLE, size, lookid) { }
	virtual ~ItemToggleButton() { }
};

/**
 * A button with an item on it.
 */
class ItemButton : public DCButton {
public:
	ItemButton(wxWindow* parent, RenderSize size, uint16_t lookid, wxWindowID id = wxID_ANY) :
		DCButton(parent, id, wxDefaultPosition, DC_BTN_NORMAL, size, lookid) { }
	virtual ~ItemButton() { }
};

/**
 * A wxListBox that can be sorted without using style wxLB_SORT.
 * wxLB_SORT does not work properly on Windows and causes errors on macOS.
 */
class SortableListBox : public wxListBox {
public:
	SortableListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	~SortableListBox();
	void Sort();

private:
	void DoSort();
};

/**
 * Go to position dialog
 * Allows entry of 3 coordinates and goes there instantly
 */
class GotoPositionDialog : public wxDialog {
public:
	GotoPositionDialog(wxWindow* parent, Editor& editor);
	~GotoPositionDialog() { }

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	Editor& editor;
	PositionCtrl* posctrl;

	DECLARE_EVENT_TABLE();
};

/**
 * Base for the item properties dialogs
 * There are two versions, one for otbmv4 maps and one for the old maps.
 * They are declared in old_properties_window / properties_window
 */
class ObjectPropertiesWindowBase : public wxDialog {
public:
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title,
		const Map* map, const Tile* tile, Item* item,
		wxPoint position = wxDefaultPosition
	);
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title,
		const Map* map, const Tile* tile, Spawn* spawn,
		wxPoint position = wxDefaultPosition
	);
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title,
		const Map* map, const Tile* tile, Creature* creature,
		wxPoint position = wxDefaultPosition
	);
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title, wxPoint position = wxDefaultPosition
	);

	Item* getItemBeingEdited();
	Creature* getCreatureBeingEdited();
	Spawn* getSpawnBeingEdited();

protected:
	const Map* edit_map;
	const Tile* edit_tile;
	Item* edit_item;
	Creature* edit_creature;
	Spawn* edit_spawn;
};

#endif
