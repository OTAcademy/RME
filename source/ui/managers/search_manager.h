#ifndef RME_SEARCH_MANAGER_H_
#define RME_SEARCH_MANAGER_H_

#include <wx/wx.h>
#include <wx/aui/aui.h>

class SearchResultWindow;

class SearchManager {
public:
	SearchManager();
	~SearchManager();

	SearchResultWindow* ShowSearchWindow();
	void HideSearchWindow();

	SearchResultWindow* GetWindow() const {
		return search_result_window;
	}

private:
	SearchResultWindow* search_result_window;
};

extern SearchManager g_search;

#endif
