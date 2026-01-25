//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DOODAD_PREVIEW_MANAGER_H_
#define RME_DOODAD_PREVIEW_MANAGER_H_

#include "app/main.h"
#include <memory>

class BaseMap;

class DoodadPreviewManager {
public:
	DoodadPreviewManager();
	~DoodadPreviewManager();

	void FillBuffer();
	BaseMap* GetBufferMap() {
		return doodad_buffer_map.get();
	}

private:
	std::unique_ptr<BaseMap> doodad_buffer_map;
};

extern DoodadPreviewManager g_doodad_preview;

#endif
