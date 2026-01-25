//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_MINIMAP_MANAGER_H_
#define RME_MINIMAP_MANAGER_H_

#include "app/main.h"

class MinimapWindow;

class MinimapManager {
public:
	MinimapManager();
	~MinimapManager();

	void Create();
	void Hide();
	void Destroy();
	void Update(bool immediate = false);
	bool IsVisible() const;

	MinimapWindow* GetWindow() {
		return minimap;
	}
	void SetWindow(MinimapWindow* mw) {
		minimap = mw;
	}

private:
	MinimapWindow* minimap;
};

extern MinimapManager g_minimap;

#endif
