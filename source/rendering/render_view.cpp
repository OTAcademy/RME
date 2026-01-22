#include "main.h"
#include "rendering/render_view.h"

#include "rendering/map_display.h"
#include "rendering/drawing_options.h"
#include "definitions.h" // For TileSize, GROUND_LAYER, MAP_MAX_LAYER
#include <algorithm> // For std::min

void RenderView::Setup(MapCanvas* canvas, const DrawingOptions& options) {
	canvas->MouseToMap(&mouse_map_x, &mouse_map_y);
	canvas->GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);

	zoom = (float)canvas->GetZoom();
	tile_size = int(TileSize / zoom); // after zoom
	floor = canvas->GetFloor();

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
}

int RenderView::getFloorAdjustment() const {
	if (floor > GROUND_LAYER) { // Underground
		return 0; // No adjustment
	} else {
		return TileSize * (GROUND_LAYER - floor);
	}
}

bool RenderView::IsTileVisible(int map_x, int map_y, int map_z) const {
	int offset = (map_z <= GROUND_LAYER)
		? (GROUND_LAYER - map_z) * TileSize
		: TileSize * (floor - map_z);
	int screen_x = (map_x * TileSize) - view_scroll_x - offset;
	int screen_y = (map_y * TileSize) - view_scroll_y - offset;
	int margin = TileSize * 3; // Account for large sprites

	if (screen_x < -margin || screen_x > screensize_x * zoom + margin || screen_y < -margin || screen_y > screensize_y * zoom + margin) {
		return false;
	}
	return true;
}

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

void RenderView::SetupGL() {
	glViewport(0, 0, screensize_x, screensize_y);

	// Enable 2D mode
	int vPort[4];

	glGetIntegerv(GL_VIEWPORT, vPort);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, vPort[2] * zoom, vPort[3] * zoom, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.375f, 0.375f, 0.0f);
}

void RenderView::ReleaseGL() {
	// Disable 2D mode
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void RenderView::Clear() {
	// Black Background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}
