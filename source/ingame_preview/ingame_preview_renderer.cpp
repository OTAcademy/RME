#include <iostream>
#include "ingame_preview/ingame_preview_renderer.h"
#include "ingame_preview/floor_visibility_calculator.h"
#include "rendering/drawers/tiles/tile_renderer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"
#include "rendering/core/light_buffer.h"
#include "rendering/utilities/light_drawer.h"
#include "map/basemap.h"
#include "ui/gui.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

namespace IngamePreview {

	IngamePreviewRenderer::IngamePreviewRenderer(TileRenderer* tile_renderer) :
		tile_renderer(tile_renderer) {
		floor_calculator = std::make_unique<FloorVisibilityCalculator>();
		sprite_batch = std::make_unique<SpriteBatch>();
		primitive_renderer = std::make_unique<PrimitiveRenderer>();
		light_buffer = std::make_unique<LightBuffer>();
		light_drawer = std::make_shared<LightDrawer>();

		sprite_batch->initialize();
		primitive_renderer->initialize();
		last_time = std::chrono::steady_clock::now();

		// Initialize opacity
		for (int z = 0; z <= MAP_MAX_LAYER; ++z) {
			floor_opacity[z] = 0.0f;
		}
	}

	IngamePreviewRenderer::~IngamePreviewRenderer() = default;

	void IngamePreviewRenderer::UpdateOpacity(double dt, int first_visible, int last_visible) {
		const float fade_speed = 4.0f; // Roughly 250ms for full fade
		for (int z = 0; z <= MAP_MAX_LAYER; ++z) {
			float target = (z >= first_visible && z <= last_visible) ? 1.0f : 0.0f;
			float current = floor_opacity[z];
			if (current < target) {
				current = std::min(target, current + static_cast<float>(dt * fade_speed));
			} else if (current > target) {
				current = std::max(target, current - static_cast<float>(dt * fade_speed));
			}
			floor_opacity[z] = current;
		}
	}

	void IngamePreviewRenderer::Render(const BaseMap& map, int viewport_x, int viewport_y, int viewport_width, int viewport_height, const Position& camera_pos, float zoom, bool lighting_enabled, uint8_t ambient_light) {
		auto now = std::chrono::steady_clock::now();
		double dt = std::chrono::duration<double>(now - last_time).count();
		last_time = now;

		int first_visible = floor_calculator->CalcFirstVisibleFloor(map, camera_pos.x, camera_pos.y, camera_pos.z);
		int last_visible = floor_calculator->CalcLastVisibleFloor(camera_pos.z);

		UpdateOpacity(dt, first_visible, last_visible);

		// Setup RenderView and DrawingOptions
		RenderView view;
		view.zoom = zoom;
		view.tile_size = TileSize;
		view.floor = camera_pos.z;
		view.screensize_x = viewport_width;
		view.screensize_y = viewport_height;
		view.viewport_x = viewport_x;
		view.viewport_y = viewport_y;

		// Proper coordinate alignment
		// We want camera_pos to be at the center of the viewport
		int offset = (camera_pos.z <= GROUND_LAYER) ? (GROUND_LAYER - camera_pos.z) * TileSize : 0;
		view.view_scroll_x = (camera_pos.x * TileSize) - offset - static_cast<int>(viewport_width * zoom / 2.0f);
		view.view_scroll_y = (camera_pos.y * TileSize) - offset - static_cast<int>(viewport_height * zoom / 2.0f);

		// Matching RME's projection (width * zoom x height * zoom)
		view.projectionMatrix = glm::ortho(0.0f, static_cast<float>(viewport_width) * zoom, static_cast<float>(viewport_height) * zoom, 0.0f, -1.0f, 1.0f);
		view.viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.375f, 0.375f, 0.0f));

		DrawingOptions options;
		options.SetIngame();
		options.show_lights = lighting_enabled;
		options.ambient_light_level = static_cast<float>(ambient_light) / 255.0f;
		options.light_intensity = light_intensity;

		// Initialize GL state
		glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		primitive_renderer->setProjectionMatrix(view.projectionMatrix);
		light_buffer->Clear();

		std::ostringstream tooltip_stream;

		// Render floors from bottom to top
		for (int z = last_visible; z >= 0; z--) {
			float alpha = floor_opacity[z];
			if (alpha <= 0.001f) {
				continue;
			}

			// Sync viewport and start batch for this floor
			sprite_batch->begin(view.projectionMatrix);
			sprite_batch->setGlobalTint(1.0f, 1.0f, 1.0f, alpha);

			// Dynamic viewport culling
			view.start_x = static_cast<int>(std::floor((view.view_scroll_x - TileSize * 2) / static_cast<float>(TileSize)));
			view.start_y = static_cast<int>(std::floor((view.view_scroll_y - TileSize * 2) / static_cast<float>(TileSize)));
			view.end_x = static_cast<int>(std::ceil((view.view_scroll_x + viewport_width * zoom + TileSize * 2) / static_cast<float>(TileSize)));
			view.end_y = static_cast<int>(std::ceil((view.view_scroll_y + viewport_height * zoom + TileSize * 2) / static_cast<float>(TileSize)));

			for (int x = view.start_x; x <= view.end_x; ++x) {
				for (int y = view.start_y; y <= view.end_y; ++y) {
					const Tile* tile = map.getTile(x, y, z);
					if (tile) {
						tile_renderer->DrawTile(*sprite_batch, *primitive_renderer, tile->location, view, options, 0, tooltip_stream);
						if (lighting_enabled) {
							tile_renderer->AddLight(tile->location, view, options, *light_buffer);
						}
					}
				}
			}
			sprite_batch->end(*g_gui.gfx.getAtlasManager());
		}

		if (lighting_enabled && light_drawer) {
			light_drawer->draw(view, options.experimental_fog, *light_buffer, options.global_light_color, options.light_intensity, options.ambient_light_level);
		}
	}

} // namespace IngamePreview
