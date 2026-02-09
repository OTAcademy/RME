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

#include "map/map.h"
#include "ui/gui.h"
#include "brushes/raw/raw_brush.h"
#include "map/tile.h"
#include "rendering/core/graphics.h"
#include "ui/gui.h"
#include "ui/browse_tile_window.h"

// ============================================================================
//

class BrowseTileListBox : public wxVListBox {
public:
	BrowseTileListBox(wxWindow* parent, wxWindowID id, Tile* tile);
	~BrowseTileListBox();

	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const;
	wxCoord OnMeasureItem(size_t index) const;
	Item* GetSelectedItem();
	void RemoveSelected();

protected:
	void UpdateItems();

	using ItemsMap = std::vector<Item*>;
	ItemsMap items;
	Tile* edit_tile;
};

BrowseTileListBox::BrowseTileListBox(wxWindow* parent, wxWindowID id, Tile* tile) :
	wxVListBox(parent, id, wxDefaultPosition, wxSize(200, 180), wxLB_MULTIPLE), edit_tile(tile) {
	UpdateItems();
}

BrowseTileListBox::~BrowseTileListBox() {
	////
}

void BrowseTileListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const {
	Item* item = items[n];

	Sprite* sprite = g_gui.gfx.getSprite(item->getClientID());
	if (sprite) {
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}

	if (IsSelected(n)) {
		item->select();
		if (HasFocus()) {
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		} else {
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
		}
	} else {
		item->deselect();
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}

	wxString label;
	label << item->getID() << " - " << wxstr(item->getName());
	dc.DrawText(label, rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord BrowseTileListBox::OnMeasureItem(size_t n) const {
	return 32;
}

Item* BrowseTileListBox::GetSelectedItem() {
	if (GetItemCount() == 0 || GetSelectedCount() == 0) {
		return nullptr;
	}

	return edit_tile->getTopSelectedItem();
}

void BrowseTileListBox::RemoveSelected() {
	if (GetItemCount() == 0 || GetSelectedCount() == 0) {
		return;
	}

	Clear();
	items.clear();

	// Delete the items from the tile
	ItemVector tile_selection = edit_tile->popSelectedItems(true);
	for (ItemVector::iterator iit = tile_selection.begin(); iit != tile_selection.end(); ++iit) {
		delete *iit;
	}

	UpdateItems();
	Refresh();
}

void BrowseTileListBox::UpdateItems() {
	items.clear();
	items.reserve(edit_tile->items.size() + (edit_tile->ground ? 1 : 0));
	for (ItemVector::reverse_iterator it = edit_tile->items.rbegin(); it != edit_tile->items.rend(); ++it) {
		items.push_back(*it);
	}

	if (edit_tile->ground) {
		items.push_back(edit_tile->ground);
	}

	SetItemCount(items.size());
}

// ============================================================================
//

BrowseTileWindow::BrowseTileWindow(wxWindow* parent, Tile* tile, wxPoint position /* = wxDefaultPosition */) :
	wxDialog(parent, wxID_ANY, "Browse Field", position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER) {
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	item_list = newd BrowseTileListBox(this, wxID_ANY, tile);
	sizer->Add(item_list, wxSizerFlags(1).Expand());

	wxString pos;
	pos << "x=" << tile->getX() << ",  y=" << tile->getY() << ",  z=" << tile->getZ();

	wxSizer* infoSizer = newd wxBoxSizer(wxVERTICAL);
	wxBoxSizer* buttons = newd wxBoxSizer(wxHORIZONTAL);
	delete_button = newd wxButton(this, wxID_REMOVE, "Delete");
	delete_button->SetToolTip("Delete selected item");
	delete_button->Enable(false);
	buttons->Add(delete_button);
	buttons->AddSpacer(5);
	select_raw_button = newd wxButton(this, wxID_FIND, "Select RAW");
	select_raw_button->SetToolTip("Select this item in RAW palette");
	select_raw_button->Enable(false);
	buttons->Add(select_raw_button);
	infoSizer->Add(buttons);
	infoSizer->AddSpacer(5);
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "Position:  " + pos), wxSizerFlags(0).Left());
	infoSizer->Add(item_count_txt = newd wxStaticText(this, wxID_ANY, "Item count:  " + i2ws(item_list->GetItemCount())), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "Protection zone:  " + b2yn(tile->isPZ())), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "No PvP:  " + b2yn(tile->getMapFlags() & TILESTATE_NOPVP)), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "No logout:  " + b2yn(tile->getMapFlags() & TILESTATE_NOLOGOUT)), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "PvP zone:  " + b2yn(tile->getMapFlags() & TILESTATE_PVPZONE)), wxSizerFlags(0).Left());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, "House:  " + b2yn(tile->isHouseTile())), wxSizerFlags(0).Left());

	sizer->Add(infoSizer, wxSizerFlags(0).Left().DoubleBorder());

	// OK/Cancel buttons
	wxSizer* btnSizer = newd wxBoxSizer(wxHORIZONTAL);
	auto okBtn = newd wxButton(this, wxID_OK, "OK");
	okBtn->SetToolTip("Confirm selection");
	btnSizer->Add(okBtn, wxSizerFlags(0).Center());
	auto cancelBtn = newd wxButton(this, wxID_CANCEL, "Cancel");
	cancelBtn->SetToolTip("Cancel");
	btnSizer->Add(cancelBtn, wxSizerFlags(0).Center());
	sizer->Add(btnSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(sizer);

	// Connect Events
	item_list->Bind(wxEVT_LISTBOX, &BrowseTileWindow::OnItemSelected, this);
	delete_button->Bind(wxEVT_BUTTON, &BrowseTileWindow::OnClickDelete, this);
	select_raw_button->Bind(wxEVT_BUTTON, &BrowseTileWindow::OnClickSelectRaw, this);
	Bind(wxEVT_BUTTON, &BrowseTileWindow::OnClickOK, this, wxID_OK);
	Bind(wxEVT_BUTTON, &BrowseTileWindow::OnClickCancel, this, wxID_CANCEL);
}

BrowseTileWindow::~BrowseTileWindow() {
}

void BrowseTileWindow::OnItemSelected(wxCommandEvent& WXUNUSED(event)) {
	const size_t count = item_list->GetSelectedCount();
	delete_button->Enable(count != 0);
	select_raw_button->Enable(count == 1);
}

void BrowseTileWindow::OnClickDelete(wxCommandEvent& WXUNUSED(event)) {
	item_list->RemoveSelected();
	item_count_txt->SetLabelText("Item count:  " + i2ws(item_list->GetItemCount()));
}

void BrowseTileWindow::OnClickSelectRaw(wxCommandEvent& WXUNUSED(event)) {
	Item* item = item_list->GetSelectedItem();
	if (item && item->getRAWBrush()) {
		g_gui.SelectBrush(item->getRAWBrush(), TILESET_RAW);
	}

	EndModal(1);
}

void BrowseTileWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	EndModal(1);
}

void BrowseTileWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}
