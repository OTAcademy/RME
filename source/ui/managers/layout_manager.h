//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_LAYOUT_MANAGER_H_
#define RME_LAYOUT_MANAGER_H_

#include "app/main.h"

class LayoutManager {
public:
	LayoutManager();
	~LayoutManager();

	/**
	 * Saves the perspective to the configuration file
	 * This is the position of all windows etc. in the editor
	 */
	void SavePerspective();

	/**
	 * Loads the stored perspective from the configuration file
	 */
	void LoadPerspective();
};

extern LayoutManager g_layout;

#endif
