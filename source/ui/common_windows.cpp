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

#include "game/materials.h"
#include "brushes/brush.h"
#include "editor/editor.h"
#include "editor/action_queue.h"

#include "game/items.h"
#include "map/map.h"
#include "game/item.h"
#include "game/complexitem.h"
#include "brushes/raw_brush.h"

#include "palette/palette_window.h"
#include "ui/gui.h"
#include "app/application.h"
#include "app/managers/version_manager.h"
#include "ui/common_windows.h"
#include "ui/positionctrl.h"
#include "ui/dialog_util.h"

#ifdef _MSC_VER
	#pragma warning(disable : 4018) // signed/unsigned mismatch
#endif

// ============================================================================
// wxListBox that can be sorted

SortableListBox::SortableListBox(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) :
	wxListBox(parent, id, pos, size, 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB) { }

SortableListBox::~SortableListBox() { }

void SortableListBox::Sort() {

	if (GetCount() == 0) {
		return;
	}

	wxASSERT_MSG(GetClientDataType() != wxClientData_Object, "Sorting a list with data of type wxClientData_Object is currently not implemented");

	DoSort();
}

void SortableListBox::DoSort() {
	size_t count = GetCount();
	int selection = GetSelection();
	wxClientDataType dataType = GetClientDataType();

	wxArrayString stringList;
	wxArrayPtrVoid dataList;

	for (size_t i = 0; i < count; ++i) {
		stringList.Add(GetString(i));
		if (dataType == wxClientData_Void) {
			dataList.Add(GetClientData(i));
		}
	}

	// Insertion sort
	for (size_t i = 0; i < count; ++i) {
		size_t j = i;
		while (j > 0 && stringList[j].CmpNoCase(stringList[j - 1]) < 0) {

			wxString tmpString = stringList[j];
			stringList[j] = stringList[j - 1];
			stringList[j - 1] = tmpString;

			if (dataType == wxClientData_Void) {
				void* tmpData = dataList[j];
				dataList[j] = dataList[j - 1];
				dataList[j - 1] = tmpData;
			}

			if (selection == j - 1) {
				selection++;
			} else if (selection == j) {
				selection--;
			}

			j--;
		}
	}

	Freeze();
	Clear();
	for (size_t i = 0; i < count; ++i) {
		if (dataType == wxClientData_Void) {
			Append(stringList[i], dataList[i]);
		} else {
			Append(stringList[i]);
		}
	}
	Thaw();

	SetSelection(selection);
}

// ============================================================================
// Object properties base

ObjectPropertiesWindowBase::ObjectPropertiesWindowBase(wxWindow* parent, wxString title, const Map* map, const Tile* tile, Item* item, wxPoint position /* = wxDefaultPosition */) :
	wxDialog(parent, wxID_ANY, title, position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	edit_map(map),
	edit_tile(tile),
	edit_item(item),
	edit_creature(nullptr),
	edit_spawn(nullptr) {
	////
}

ObjectPropertiesWindowBase::ObjectPropertiesWindowBase(wxWindow* parent, wxString title, const Map* map, const Tile* tile, Creature* creature, wxPoint position /* = wxDefaultPosition */) :
	wxDialog(parent, wxID_ANY, title, position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	edit_map(map),
	edit_tile(tile),
	edit_item(nullptr),
	edit_creature(creature),
	edit_spawn(nullptr) {
	////
}

ObjectPropertiesWindowBase::ObjectPropertiesWindowBase(wxWindow* parent, wxString title, const Map* map, const Tile* tile, Spawn* spawn, wxPoint position /* = wxDefaultPosition */) :
	wxDialog(parent, wxID_ANY, title, position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	edit_map(map),
	edit_tile(tile),
	edit_item(nullptr),
	edit_creature(nullptr),
	edit_spawn(spawn) {
	////
}

ObjectPropertiesWindowBase::ObjectPropertiesWindowBase(wxWindow* parent, wxString title, wxPoint position /* = wxDefaultPosition */) :
	wxDialog(parent, wxID_ANY, title, position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER) {
	////
}

Item* ObjectPropertiesWindowBase::getItemBeingEdited() {
	return edit_item;
}

// ============================================================================
// Go To Position Dialog
// Jump to a position on the map by entering XYZ coordinates

BEGIN_EVENT_TABLE(GotoPositionDialog, wxDialog)
EVT_BUTTON(wxID_OK, GotoPositionDialog::OnClickOK)
EVT_BUTTON(wxID_CANCEL, GotoPositionDialog::OnClickCancel)
END_EVENT_TABLE()

GotoPositionDialog::GotoPositionDialog(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, "Go To Position", wxDefaultPosition, wxDefaultSize),
	editor(editor) {
	Map& map = editor.map;

	// create topsizer
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	posctrl = newd PositionCtrl(this, "Destination", map.getWidth() / 2, map.getHeight() / 2, GROUND_LAYER, map.getWidth(), map.getHeight());
	sizer->Add(posctrl, 0, wxTOP | wxLEFT | wxRIGHT, 20);

	// OK/Cancel buttons
	wxSizer* tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
	tmpsizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, 0, wxALL | wxCENTER, 20); // Border to top too

	SetSizerAndFit(sizer);
	Centre(wxBOTH);
}

void GotoPositionDialog::OnClickCancel(wxCommandEvent&) {
	EndModal(0);
}

void GotoPositionDialog::OnClickOK(wxCommandEvent&) {
	g_gui.SetScreenCenterPosition(posctrl->GetPosition());
	EndModal(1);
}
