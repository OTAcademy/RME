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

#ifndef RME_TILESET_WINDOW_H_
#define RME_TILESET_WINDOW_H_

#include "app/main.h"
#include "ui/properties/object_properties_base.h"

#include "ui/controls/item_buttons.h"

class ContainerItemButton;
class ContainerItemPopupMenu;

class TilesetWindow : public ObjectPropertiesWindowBase {
public:
	TilesetWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint = wxDefaultPosition);

	void OnChangePalette(wxCommandEvent& event);

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

protected:
	// tileset
	wxChoice* palette_field;
	wxChoice* tileset_field;
};

#endif
