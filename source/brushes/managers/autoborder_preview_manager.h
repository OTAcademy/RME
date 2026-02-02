//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_AUTOBORDER_PREVIEW_MANAGER_H_
#define RME_AUTOBORDER_PREVIEW_MANAGER_H_

#include "app/main.h"
#include <memory>
#include <vector>
#include "map/position.h"

class BaseMap;
class Editor;
class Map;

class AutoborderPreviewManager {
public:
	AutoborderPreviewManager();
	~AutoborderPreviewManager();

	void Update(Editor& editor, const Position& pos, bool alt_pressed);
	BaseMap* GetBufferMap() {
		return preview_buffer_map.get();
	}

	void Clear();

private:
	std::unique_ptr<BaseMap> preview_buffer_map;
	Position last_pos;

	bool CheckPreconditions(const Position& pos);
	void CopyMapArea(Editor& editor, const Position& pos);
	void SimulateBrush(Editor& editor, const Position& pos, const std::vector<Position>& tilestodraw, bool alt_pressed);
	void ApplyBorders(const std::vector<Position>& tilestodraw, const std::vector<Position>& tilestoborder);
	void PruneUnchanged(Editor& editor, const Position& pos);
};

extern AutoborderPreviewManager g_autoborder_preview;

#endif
