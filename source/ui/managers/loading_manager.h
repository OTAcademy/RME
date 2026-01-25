//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_LOADING_MANAGER_H_
#define RME_LOADING_MANAGER_H_

#include "app/main.h"
#include <wx/progdlg.h>

class LoadingManager {
public:
	LoadingManager();
	~LoadingManager();

	/**
	 * Creates a loading bar with the specified message, title is always "Loading"
	 * The default scale is 0 - 100
	 */
	void CreateLoadBar(wxString message, bool canCancel = false);

	/**
	 * Sets how much of the load has completed, the scale can be set with
	 * SetLoadScale.
	 * If this returns false, the user has hit the quit button and you should
	 * abort the loading.
	 */
	bool SetLoadDone(int32_t done, const wxString& newMessage = "");

	/**
	 * Sets the scale of the loading bar.
	 * Calling this with (50, 80) means that setting 50 as 'done',
	 * it will display as 0% loaded, 80 will display as 100% loaded.
	 */
	void SetLoadScale(int32_t from, int32_t to);

	/**
	 * Destroys (hides) the current loading bar.
	 */
	void DestroyLoadBar();

private:
	wxString progressText;
	wxGenericProgressDialog* progressBar;

	int32_t progressFrom;
	int32_t progressTo;
	int32_t currentProgress;
};

extern LoadingManager g_loading;

#endif
