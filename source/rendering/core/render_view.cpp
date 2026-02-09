#include "app/main.h"
#include "rendering/core/render_view.h"

#include "rendering/ui/map_display.h"
#include "rendering/core/drawing_options.h"
#include "app/definitions.h" // For TileSize, GROUND_LAYER, MAP_MAX_LAYER
#include <algorithm> // For std::min

void RenderView::Setup(MapCanvas* canvas, const DrawingOptions& options) {
	canvas->MouseToMap(&mouse_map_x, &mouse_map_y);
	canvas->GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);
	viewport_x = 0;
	viewport_y = 0;

	zoom = (float)canvas->GetZoom();
	tile_size = std::max(1, (int)(TileSize / zoom)); // after zoom
	floor = canvas->GetFloor();
	camera_pos.z = floor;

	if (options.show_all_floors) {
		if (floor <= GROUND_LAYER) {
			start_z = GROUND_LAYER;
		} else {
			start_z = std::min(MAP_MAX_LAYER, floor + 2);
		}
	} else {
		start_z = floor;
	}

	end_z = floor;
	superend_z = (floor > GROUND_LAYER ? 8 : 0);

	start_x = view_scroll_x / TileSize;
	start_y = view_scroll_y / TileSize;

	if (floor > GROUND_LAYER) {
		start_x -= 2;
		start_y -= 2;
	}

	end_x = start_x + screensize_x / tile_size + 2;
	end_y = start_y + screensize_y / tile_size + 2;

	// Calculate logical dimensions
	logical_width = screensize_x * zoom;
	logical_height = screensize_y * zoom;
}

int RenderView::getFloorAdjustment() const {
	if (floor > GROUND_LAYER) { // Underground
		return 0; // No adjustment
	} else {
		return TileSize * (GROUND_LAYER - floor);
	}
}

bool RenderView::IsTileVisible(int map_x, int map_y, int map_z, int& out_x, int& out_y) const {
	int offset = (map_z <= GROUND_LAYER)
		? (GROUND_LAYER - map_z) * TileSize
		: TileSize * (floor - map_z);
	out_x = (map_x * TileSize) - view_scroll_x - offset;
	out_y = (map_y * TileSize) - view_scroll_y - offset;
	const int margin = PAINTERS_ALGORITHM_SAFETY_MARGIN_PIXELS;

	// Use cached logical dimensions
	if (out_x < -margin || out_x > logical_width + margin || out_y < -margin || out_y > logical_height + margin) {
		return false;
	}
	return true;
}

bool RenderView::IsPixelVisible(int draw_x, int draw_y, int margin) const {
	// Logic matches IsTileVisible but uses pre-calculated draw coordinates.
	// screensize_x * zoom gives the logical viewport size (since TileSize is constant 32).
	// See SetupGL: glOrtho(0, width * zoom, ...)

	// Use cached logical dimensions
	if (draw_x + TileSize + margin < 0 || draw_x - margin > logical_width || draw_y + TileSize + margin < 0 || draw_y - margin > logical_height) {
		return false;
	}
	return true;
}

bool RenderView::IsRectVisible(int draw_x, int draw_y, int width, int height, int margin) const {
	if (draw_x + width + margin < 0 || draw_x - margin > logical_width || draw_y + height + margin < 0 || draw_y - margin > logical_height) {
		return false;
	}
	return true;
}

bool RenderView::IsRectFullyInside(int draw_x, int draw_y, int width, int height) const {
	return (draw_x >= 0 && draw_x + width <= logical_width && draw_y >= 0 && draw_y + height <= logical_height);
}

void RenderView::getScreenPosition(int map_x, int map_y, int map_z, int& out_x, int& out_y) const {
	int offset = (map_z <= GROUND_LAYER)
		? (GROUND_LAYER - map_z) * TileSize
		: TileSize * (floor - map_z);
	out_x = (map_x * TileSize) - view_scroll_x - offset;
	out_y = (map_y * TileSize) - view_scroll_y - offset;
}

#include <glm/gtc/matrix_transform.hpp>

void RenderView::SetupGL() {
	glViewport(viewport_x, viewport_y, screensize_x, screensize_y);

	// Calculate Projection
	// glOrtho(0, vPort[2] * zoom, vPort[3] * zoom, 0, -1, 1);
	// Equivalent: 0 -> width*zoom, height*zoom -> 0

	int width = screensize_x;
	int height = screensize_y;

	projectionMatrix = glm::ortho(0.0f, width * zoom, height * zoom, 0.0f, -1.0f, 1.0f);

	// Calculate ModelView
	// glTranslatef(0.375f, 0.375f, 0.0f);
	viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.375f, 0.375f, 0.0f));
}

void RenderView::ReleaseGL() {
	// No legacy matrix stack to cleanup
}

void RenderView::Clear() {
	// Black Background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// glLoadIdentity(); // Legacy
	// Blending and State management is now handled by individual renderers
}
