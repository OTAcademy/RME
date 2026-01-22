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
