#ifndef RME_UI_MENUBAR_SEARCH_HANDLER_H_
#define RME_UI_MENUBAR_SEARCH_HANDLER_H_

#include <wx/wx.h>

class MainFrame;

class SearchHandler {
public:
	SearchHandler(MainFrame* frame);

	void OnSearchForItem(wxCommandEvent& event);
	void OnReplaceItems(wxCommandEvent& event);

	// Map searches
	void OnSearchForStuffOnMap(wxCommandEvent& event);
	void OnSearchForUniqueOnMap(wxCommandEvent& event);
	void OnSearchForActionOnMap(wxCommandEvent& event);
	void OnSearchForContainerOnMap(wxCommandEvent& event);
	void OnSearchForWriteableOnMap(wxCommandEvent& event);

	// Selection searches
	void OnSearchForStuffOnSelection(wxCommandEvent& event);
	void OnSearchForUniqueOnSelection(wxCommandEvent& event);
	void OnSearchForActionOnSelection(wxCommandEvent& event);
	void OnSearchForContainerOnSelection(wxCommandEvent& event);
	void OnSearchForWriteableOnSelection(wxCommandEvent& event);
	void OnSearchForItemOnSelection(wxCommandEvent& event);
	void OnReplaceItemsOnSelection(wxCommandEvent& event);
	void OnRemoveItemOnSelection(wxCommandEvent& event);

private:
	void SearchItems(bool unique, bool action, bool container, bool writable, bool onSelection = false);

	MainFrame* frame;
};

#endif
