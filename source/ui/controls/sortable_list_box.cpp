//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/controls/sortable_list_box.h"

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

	if (selection != wxNOT_FOUND) {
		SetSelection(selection);
	}
}
