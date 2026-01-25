//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DOODAD_PREVIEW_MANAGER_H_
#define RME_DOODAD_PREVIEW_MANAGER_H_

#include "app/main.h"

class BaseMap;

class DoodadPreviewManager {
public:
	DoodadPreviewManager();
	~DoodadPreviewManager();

	void FillBuffer();
	BaseMap* GetBufferMap() {
		return doodad_buffer_map;
	}

private:
	BaseMap* doodad_buffer_map;
};

extern DoodadPreviewManager g_doodad_preview;

#endif
