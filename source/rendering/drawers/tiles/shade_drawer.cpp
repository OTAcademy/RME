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

void ShadeDrawer::draw(const RenderView& view, const DrawingOptions& options) {
	if (view.start_z != view.end_z && options.show_shade) {
		bool only_colors = options.show_as_minimap || options.show_only_colors;
		// Draw shade
		if (!only_colors) {
			glDisable(GL_TEXTURE_2D);
		}

		glColor4ub(0, 0, 0, 128);
		glBegin(GL_QUADS);
		glVertex2f(0, int(view.screensize_y * view.zoom));
		glVertex2f(int(view.screensize_x * view.zoom), int(view.screensize_y * view.zoom));
		glVertex2f(int(view.screensize_x * view.zoom), 0);
		glVertex2f(0, 0);
		glEnd();

		if (!only_colors) {
			glEnable(GL_TEXTURE_2D);
		}
	}
}
