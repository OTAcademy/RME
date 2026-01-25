//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/controls/sortable_list_box.h"
#include <algorithm>
#include <vector>
#include <utility>

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
	if (count == 0) {
		return;
	}

	int selection_index = GetSelection();
	wxString selection_string;
	if (selection_index != wxNOT_FOUND) {
		selection_string = GetString(selection_index);
	}

	std::vector<std::pair<wxString, void*>> items;
	items.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		items.emplace_back(GetString(i), GetClientData(i));
	}

	std::stable_sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
		return a.first.CmpNoCase(b.first) < 0;
	});

	Freeze();
	Clear();
	for (const auto& item : items) {
		Append(item.first, item.second);
	}
	Thaw();

	if (selection_index != wxNOT_FOUND) {
		int new_selection = FindString(selection_string);
		if (new_selection != wxNOT_FOUND) {
			SetSelection(new_selection);
		}
	}
}
