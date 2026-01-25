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

void MapLayerDrawer::Draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, int map_z, bool live_client, const RenderView& view, const DrawingOptions& options, LightBuffer& light_buffer, std::ostringstream& tooltip) {
	int nd_start_x = view.start_x & ~3;
	int nd_start_y = view.start_y & ~3;
	int nd_end_x = (view.end_x & ~3) + 4;
	int nd_end_y = (view.end_y & ~3) + 4;

	for (int nd_map_x = nd_start_x; nd_map_x <= nd_end_x; nd_map_x += 4) {
		for (int nd_map_y = nd_start_y; nd_map_y <= nd_end_y; nd_map_y += 4) {
			QTreeNode* nd = editor->map.getLeaf(nd_map_x, nd_map_y);
			if (!nd) {
				if (live_client) {
					nd = editor->map.createLeaf(nd_map_x, nd_map_y);
					nd->setVisible(false, false);
				} else {
					continue;
				}
			}

			if (!live_client || nd->isVisible(map_z > GROUND_LAYER)) {
				for (int map_x = 0; map_x < 4; ++map_x) {
					for (int map_y = 0; map_y < 4; ++map_y) {
						TileLocation* location = nd->getTile(map_x, map_y, map_z);
						tile_renderer->DrawTile(sprite_batch, primitive_renderer, location, view, options, options.current_house_id, tooltip);
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
}
