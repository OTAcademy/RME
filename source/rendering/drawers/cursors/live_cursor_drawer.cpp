#include "app/main.h"

#include "rendering/drawers/cursors/live_cursor_drawer.h"

#include "rendering/core/sprite_batch.h"
#include "rendering/core/render_view.h"
#include "editor/editor.h"
#include "live/live_socket.h"
#include "rendering/core/drawing_options.h"
#include "rendering/core/graphics.h"
#include "ui/gui.h"

void LiveCursorDrawer::draw(SpriteBatch& sprite_batch, const RenderView& view, Editor& editor, const DrawingOptions& options) {
	if (options.ingame || !editor.live_manager.IsLive()) {
		return;
	}

	LiveSocket& live = editor.live_manager.GetSocket();
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

		glm::vec4 color(
			cursor.color.Red() / 255.0f,
			cursor.color.Green() / 255.0f,
			cursor.color.Blue() / 255.0f,
			cursor.color.Alpha() / 255.0f
		);

		if (g_gui.gfx.ensureAtlasManager()) {
			sprite_batch.drawRect(draw_x, draw_y, (float)TileSize, (float)TileSize, color, *g_gui.gfx.getAtlasManager());
		}
	}
}
