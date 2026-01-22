#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/selection_drawer.h"
#include "rendering/render_view.h"
#include "rendering/drawing_options.h"
#include "rendering/map_display.h"

void SelectionDrawer::draw(const RenderView& view, const MapCanvas* canvas, const DrawingOptions& options) {
	if (options.ingame) {
		return;
	}

	// Draw bounding box

	int last_click_rx = canvas->last_click_abs_x - view.view_scroll_x;
	int last_click_ry = canvas->last_click_abs_y - view.view_scroll_y;
	double cursor_rx = canvas->cursor_x * view.zoom;
	double cursor_ry = canvas->cursor_y * view.zoom;

	double lines[4][4];

	lines[0][0] = last_click_rx;
	lines[0][1] = last_click_ry;
	lines[0][2] = cursor_rx;
	lines[0][3] = last_click_ry;

	lines[1][0] = cursor_rx;
	lines[1][1] = last_click_ry;
	lines[1][2] = cursor_rx;
	lines[1][3] = cursor_ry;

	lines[2][0] = cursor_rx;
	lines[2][1] = cursor_ry;
	lines[2][2] = last_click_rx;
	lines[2][3] = cursor_ry;

	lines[3][0] = last_click_rx;
	lines[3][1] = cursor_ry;
	lines[3][2] = last_click_rx;
	lines[3][3] = last_click_ry;

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xf0);
	glLineWidth(1.0);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_LINES);
	for (int i = 0; i < 4; i++) {
		glVertex2f(lines[i][0], lines[i][1]);
		glVertex2f(lines[i][2], lines[i][3]);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}
