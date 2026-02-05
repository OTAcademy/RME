//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "app/definitions.h"
#include "rendering/drawers/map_layer_drawer.h"
#include "rendering/drawers/tiles/tile_renderer.h"
#include "rendering/drawers/overlays/grid_drawer.h"
#include "editor/editor.h"
#include "live/live_client.h"
#include "map/map.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include "rendering/core/light_buffer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"

MapLayerDrawer::MapLayerDrawer(TileRenderer* tile_renderer, GridDrawer* grid_drawer, Editor* editor) :
	tile_renderer(tile_renderer),
	grid_drawer(grid_drawer),
	editor(editor) {
}

MapLayerDrawer::~MapLayerDrawer() {
}

void MapLayerDrawer::Draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, int map_z, bool live_client, const RenderView& view, const DrawingOptions& options, LightBuffer& light_buffer) {
	int nd_start_x = view.start_x & ~3;
	int nd_start_y = view.start_y & ~3;
	int nd_end_x = (view.end_x & ~3) + 4;
	int nd_end_y = (view.end_y & ~3) + 4;

	// Optimization: Pre-calculate offset and base coordinates
	// IsTileVisible does this for every tile, but it's constant per layer/frame.
	// We also skip IsTileVisible because visitLeaves already bounds us to the visible area (with 4-tile alignment),
	// which is well within IsTileVisible's 6-tile margin.
	int offset = (map_z <= GROUND_LAYER)
		? (GROUND_LAYER - map_z) * TileSize
		: TileSize * (view.floor - map_z);

	int base_screen_x = -view.view_scroll_x - offset;
	int base_screen_y = -view.view_scroll_y - offset;

	// ND visibility

	if (live_client) {
		for (int nd_map_x = nd_start_x; nd_map_x <= nd_end_x; nd_map_x += 4) {
			for (int nd_map_y = nd_start_y; nd_map_y <= nd_end_y; nd_map_y += 4) {
				MapNode* nd = editor->map.getLeaf(nd_map_x, nd_map_y);
				if (!nd) {
					nd = editor->map.createLeaf(nd_map_x, nd_map_y);
					nd->setVisible(false, false);
				}

				if (nd->isVisible(map_z > GROUND_LAYER)) {
					int node_draw_x = nd_map_x * TileSize + base_screen_x;
					int node_draw_y = nd_map_y * TileSize + base_screen_y;

					for (int map_x = 0; map_x < 4; ++map_x) {
						for (int map_y = 0; map_y < 4; ++map_y) {
							// Calculate draw coordinates directly
							int draw_x = node_draw_x + (map_x * TileSize);
							int draw_y = node_draw_y + (map_y * TileSize);

							TileLocation* location = nd->getTile(map_x, map_y, map_z);

							tile_renderer->DrawTile(sprite_batch, primitive_renderer, location, view, options, options.current_house_id, draw_x, draw_y);
							// draw light, but only if not zoomed too far
							if (location && options.isDrawLight() && view.zoom <= 10.0) {
								tile_renderer->AddLight(location, view, options, light_buffer);
							}
						}
					}
				} else {
					if (!nd->isRequested(map_z > GROUND_LAYER)) {
						// Request the node
						if (editor->live_manager.GetClient()) {
							editor->live_manager.GetClient()->queryNode(nd_map_x, nd_map_y, map_z > GROUND_LAYER);
						}
						nd->setRequested(map_z > GROUND_LAYER, true);
					}
					grid_drawer->DrawNodeLoadingPlaceholder(sprite_batch, nd_map_x, nd_map_y, view);
				}
			}
		}
	} else {
		editor->map.visitLeaves(nd_start_x, nd_start_y, nd_end_x, nd_end_y, [&](MapNode* nd, int nd_map_x, int nd_map_y) {
			int node_draw_x = nd_map_x * TileSize + base_screen_x;
			int node_draw_y = nd_map_y * TileSize + base_screen_y;

			for (int map_x = 0; map_x < 4; ++map_x) {
				for (int map_y = 0; map_y < 4; ++map_y) {
					// Calculate draw coordinates directly
					int draw_x = node_draw_x + (map_x * TileSize);
					int draw_y = node_draw_y + (map_y * TileSize);

					TileLocation* location = nd->getTile(map_x, map_y, map_z);

					tile_renderer->DrawTile(sprite_batch, primitive_renderer, location, view, options, options.current_house_id, draw_x, draw_y);
					// draw light, but only if not zoomed too far
					if (location && options.isDrawLight() && view.zoom <= 10.0) {
						tile_renderer->AddLight(location, view, options, light_buffer);
					}
				}
			}
		});
	}
}
