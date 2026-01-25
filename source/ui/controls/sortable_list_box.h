//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_SortableListBox_H
#define RME_SortableListBox_H

#include "app/main.h"

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

#endif
