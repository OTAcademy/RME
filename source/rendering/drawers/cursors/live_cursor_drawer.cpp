#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/cursors/live_cursor_drawer.h"
#include "rendering/core/render_view.h"
#include "editor.h"
#include "live_socket.h"
#include "rendering/core/drawing_options.h"

void LiveCursorDrawer::draw(const RenderView& view, Editor& editor, const DrawingOptions& options) {
	if (options.ingame || !editor.IsLive()) {
		return;
	}

	LiveSocket& live = editor.GetLive();
	for (LiveCursor& cursor : live.getCursorList()) {
		if (cursor.pos.z <= GROUND_LAYER && view.floor > GROUND_LAYER) {
			continue;
		}

		if (cursor.pos.z > GROUND_LAYER && view.floor <= 8) {
			continue;
		}

		if (cursor.pos.z < view.floor) {
			cursor.color = wxColor(
				cursor.color.Red(),
				cursor.color.Green(),
				cursor.color.Blue(),
				std::max<uint8_t>(cursor.color.Alpha() / 2, 64)
			);
		}

		int offset;
		if (cursor.pos.z <= GROUND_LAYER) {
			offset = (GROUND_LAYER - cursor.pos.z) * TileSize;
		} else {
			offset = TileSize * (view.floor - cursor.pos.z);
		}

		float draw_x = ((cursor.pos.x * TileSize) - view.view_scroll_x) - offset;
		float draw_y = ((cursor.pos.y * TileSize) - view.view_scroll_y) - offset;

		// glColor(cursor.color); // Replaced with direct GL call since helper might not be valid here
		// Assuming cursor.color is wxColor
		glColor4ub(cursor.color.Red(), cursor.color.Green(), cursor.color.Blue(), cursor.color.Alpha());

		glBegin(GL_QUADS);
		glVertex2f(draw_x, draw_y);
		glVertex2f(draw_x + TileSize, draw_y);
		glVertex2f(draw_x + TileSize, draw_y + TileSize);
		glVertex2f(draw_x, draw_y + TileSize);
		glEnd();
	}
}
