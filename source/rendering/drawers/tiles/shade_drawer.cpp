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

#include "rendering/core/batch_renderer.h"

void ShadeDrawer::draw(const RenderView& view, const DrawingOptions& options) {
	if (view.start_z != view.end_z && options.show_shade) {
		glm::vec4 color(0.0f, 0.0f, 0.0f, 128.0f / 255.0f);
		float w = view.screensize_x * view.zoom;
		float h = view.screensize_y * view.zoom;

		BatchRenderer::DrawQuad(glm::vec2(0, 0), glm::vec2(w, h), color);
	}
}
