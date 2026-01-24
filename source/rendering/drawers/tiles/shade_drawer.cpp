#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/tiles/shade_drawer.h"

ShadeDrawer::ShadeDrawer() {
}

ShadeDrawer::~ShadeDrawer() {
}

#include "rendering/core/sprite_batch.h"
#include "rendering/core/graphics.h"
#include "gui.h"

void ShadeDrawer::draw(SpriteBatch& sprite_batch, const RenderView& view, const DrawingOptions& options) {
	if (view.start_z != view.end_z && options.show_shade) {
		glm::vec4 color(0.0f, 0.0f, 0.0f, 128.0f / 255.0f);
		float w = view.screensize_x * view.zoom;
		float h = view.screensize_y * view.zoom;

		if (g_gui.gfx.ensureAtlasManager()) {
			sprite_batch.drawRect(0.0f, 0.0f, w, h, color, *g_gui.gfx.getAtlasManager());
		}
	}
}
