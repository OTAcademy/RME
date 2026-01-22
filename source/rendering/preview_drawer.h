#ifndef RME_PREVIEW_DRAWER_H_
#define RME_PREVIEW_DRAWER_H_

#include "rendering/render_view.h"
#include "rendering/drawing_options.h"
#include <cstdint>

class MapCanvas;
class Editor;
class ItemDrawer;
class SpriteDrawer;
class CreatureDrawer;

class PreviewDrawer {
public:
	PreviewDrawer();
	~PreviewDrawer();

	void draw(MapCanvas* canvas, const RenderView& view, int map_z, const DrawingOptions& options, Editor& editor, ItemDrawer* item_drawer, SpriteDrawer* sprite_drawer, CreatureDrawer* creature_drawer, uint32_t current_house_id);
};

#endif
