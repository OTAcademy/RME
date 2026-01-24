#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/overlays/selection_drawer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include "rendering/ui/map_display.h"
#include "rendering/core/graphics.h"
#include "gui.h"

void SelectionDrawer::draw(SpriteBatch& sprite_batch, const RenderView& view, const MapCanvas* canvas, const DrawingOptions& options) {
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

	// glLineStipple is deprecated in Core 4.5.
	// We will draw solid lines for now. If stipple is needed, we need a shader solution.

	glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);

	if (g_gui.gfx.ensureAtlasManager()) {
		const AtlasManager& atlas = *g_gui.gfx.getAtlasManager();
		for (int i = 0; i < 4; i++) {
			// BatchRenderer::DrawLine(glm::vec2(lines[i][0], lines[i][1]), glm::vec2(lines[i][2], lines[i][3]), color);
			float x1 = lines[i][0];
			float y1 = lines[i][1];
			float x2 = lines[i][2];
			float y2 = lines[i][3];

			// Assuming axis aligned lines for selection box
			float w = std::abs(x2 - x1);
			float h = std::abs(y2 - y1);
			if (w == 0) {
				w = 1.0f; // Vertical line
			}
			if (h == 0) {
				h = 1.0f; // Horizontal line
			}

			// Need to handle non-axis aligned lines? Selection box usually is axis aligned.
			// But if rotation involves... SelectionDrawer logic above suggests it follows 4 corners.
			// Wait, lines[0] is (last_click_rx, last_click_ry) to (cursor_rx, last_click_ry). That is Horizontal.
			// lines[1] is (cursor_rx, last_click_ry) to (cursor_rx, cursor_ry). That is Vertical.
			// It is axis aligned.

			float x = std::min(x1, x2);
			float y = std::min(y1, y2);

			sprite_batch.drawRect(x, y, w, h, color, atlas);
		}
	}
}
