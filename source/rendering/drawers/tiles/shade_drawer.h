#ifndef RME_SHADE_DRAWER_H_
#define RME_SHADE_DRAWER_H_

#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"

class SpriteBatch;

class ShadeDrawer {
public:
	ShadeDrawer();
	~ShadeDrawer();

	void draw(SpriteBatch& sprite_batch, const RenderView& view, const DrawingOptions& options);
};

#endif
