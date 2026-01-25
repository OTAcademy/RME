//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_STATUS_MANAGER_H_
#define RME_STATUS_MANAGER_H_

#include "app/main.h"

class StatusManager {
public:
	void SetTitle(const wxString& title);
	void UpdateTitle();
	void SetStatusText(const wxString& text);
};

extern StatusManager g_status;

#endif
