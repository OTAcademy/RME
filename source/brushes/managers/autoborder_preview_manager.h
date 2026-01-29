//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_AUTOBORDER_PREVIEW_MANAGER_H_
#define RME_AUTOBORDER_PREVIEW_MANAGER_H_

#include "app/main.h"
#include <memory>
#include "map/position.h"

class BaseMap;
class Editor;
class Map;

class AutoborderPreviewManager {
public:
	AutoborderPreviewManager();
	~AutoborderPreviewManager();

	void Update(Editor& editor, const Position& pos);
	BaseMap* GetBufferMap() {
		return preview_buffer_map.get();
	}

	void Clear();

private:
	std::unique_ptr<BaseMap> preview_buffer_map;
	Position last_pos;
	int last_z;
};

extern AutoborderPreviewManager g_autoborder_preview;

#endif
