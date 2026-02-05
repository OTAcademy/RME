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

#include <wx/grid.h>

#include "map/tile.h"
#include "game/item.h"
#include "game/complexitem.h"
#include "game/town.h"
#include "game/house.h"
#include "map/map.h"
#include "editor/editor.h"
#include "game/creature.h"
#include "game/materials.h"
#include "map/tileset.h"

#include "ui/gui.h"
#include "ui/dialog_util.h"
#include "app/application.h"
#include "ui/tileset_window.h"
#include "ui/properties/container_properties_window.h"

// ============================================================================
// Tileset Window

static constexpr int OUTFIT_COLOR_MAX = 133;

TilesetWindow::TilesetWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Move to Tileset", map, tile_parent, item, pos),
	palette_field(nullptr),
	tileset_field(nullptr) {
	ASSERT(edit_item);

	Bind(wxEVT_BUTTON, &TilesetWindow::OnClickOK, this, wxID_OK);
	Bind(wxEVT_BUTTON, &TilesetWindow::OnClickCancel, this, wxID_CANCEL);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxString description = "Move to Tileset";

	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, description);

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Palette"));
	palette_field = newd wxChoice(this, wxID_ANY);
	palette_field->SetToolTip("Select the palette category");

	palette_field->Append("Terrain", newd int(TILESET_TERRAIN));
	palette_field->Append("Collections", newd int(TILESET_COLLECTION));
	palette_field->Append("Doodad", newd int(TILESET_DOODAD));
	palette_field->Append("Item", newd int(TILESET_ITEM));
	palette_field->Append("Raw", newd int(TILESET_RAW));
	palette_field->SetSelection(3);

	palette_field->Bind(wxEVT_CHOICE, &TilesetWindow::OnChangePalette, this);
	subsizer->Add(palette_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Tileset"));
	tileset_field = newd wxChoice(this, wxID_ANY);
	tileset_field->SetToolTip("Select the target tileset");

	for (const auto& tileset : GetSortedTilesets(g_materials.tilesets)) {
		tileset_field->Append(wxstr(tileset->name), newd std::string(tileset->name));
	}
	tileset_field->SetSelection(0);
	subsizer->Add(tileset_field, wxSizerFlags(1).Expand());

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));

	wxSizer* subsizer_ = newd wxBoxSizer(wxHORIZONTAL);
	wxButton* okBtn = newd wxButton(this, wxID_OK, "OK");
	okBtn->SetToolTip("Confirm changes");
	subsizer_->Add(okBtn, wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));

	wxButton* cancelBtn = newd wxButton(this, wxID_CANCEL, "Cancel");
	cancelBtn->SetToolTip("Discard changes");
	subsizer_->Add(cancelBtn, wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(subsizer_, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);

	Bind(wxEVT_BUTTON, &TilesetWindow::OnClickOK, this, wxID_OK);
	Bind(wxEVT_BUTTON, &TilesetWindow::OnClickCancel, this, wxID_CANCEL);
}

void TilesetWindow::OnChangePalette(wxCommandEvent& WXUNUSED(event)) {
	tileset_field->Clear();

	for (const auto& tileset : GetSortedTilesets(g_materials.tilesets)) {
		if (!tileset->getCategory(static_cast<TilesetCategoryType>(*static_cast<int*>(palette_field->GetClientData(palette_field->GetSelection()))))->brushlist.empty()) {
			tileset_field->Append(wxstr(tileset->name), newd std::string(tileset->name));
		}
	}

	tileset_field->SetSelection(0);
}

void TilesetWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	if (edit_item) {
		TilesetCategoryType categoryType = TilesetCategoryType(*reinterpret_cast<int*>(palette_field->GetClientData(palette_field->GetSelection())));
		std::string tilesetName = *static_cast<std::string*>(tileset_field->GetClientData(tileset_field->GetSelection()));

		g_materials.addToTileset(tilesetName, edit_item->getID(), categoryType);
		g_gui.SetStatusText("'" + std::string(edit_item->getName()) + "' added to tileset '" + tilesetName + "'");
	}
	EndModal(1);
}

void TilesetWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	// Just close this window
	EndModal(0);
}
