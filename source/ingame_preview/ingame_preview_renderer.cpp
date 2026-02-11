#include "ingame_preview/ingame_preview_renderer.h"
#include "ingame_preview/floor_visibility_calculator.h"
#include "rendering/drawers/tiles/tile_renderer.h"
#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"
#include "rendering/core/light_buffer.h"
#include "rendering/utilities/light_drawer.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/drawers/entities/creature_name_drawer.h"
#include "rendering/drawers/entities/sprite_drawer.h"
#include "game/outfit.h"
#include "map/basemap.h"
#include "map/tile.h"
#include "game/creature.h"
#include "ui/gui.h"
#include "rendering/core/text_renderer.h"
#include <glad/glad.h>
#include <nanovg.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <algorithm> // For std::max
#include <cstdlib> // For std::abs

namespace IngamePreview {

	IngamePreviewRenderer::IngamePreviewRenderer(TileRenderer* tile_renderer) :
		tile_renderer(tile_renderer) {
		floor_calculator = std::make_unique<FloorVisibilityCalculator>();
		sprite_batch = std::make_unique<SpriteBatch>();
		primitive_renderer = std::make_unique<PrimitiveRenderer>();
		light_buffer = std::make_unique<LightBuffer>();
		light_drawer = std::make_shared<LightDrawer>();
		creature_drawer = std::make_unique<CreatureDrawer>();
		creature_name_drawer = std::make_unique<CreatureNameDrawer>();
		sprite_drawer = std::make_unique<SpriteDrawer>();

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

	void IngamePreviewRenderer::Render(NVGcontext* vg, const BaseMap& map, int viewport_x, int viewport_y, int viewport_width, int viewport_height, const Position& camera_pos, float zoom, bool lighting_enabled, uint8_t ambient_light, const Outfit& preview_outfit, Direction preview_direction, int animation_phase, int offset_x, int offset_y) {
		// CRITICAL: Update animation time for all sprite animations to work
		g_gui.gfx.updateTime();

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
		view.camera_pos = camera_pos;
		view.viewport_x = viewport_x;
		view.viewport_y = viewport_y;

		// Initialize cached logical dimensions (required for visibility culling)
		view.logical_width = viewport_width * zoom;
		view.logical_height = viewport_height * zoom;

		// Proper coordinate alignment
		// We want camera_pos to be at the center of the viewport
		int offset = (camera_pos.z <= GROUND_LAYER) ? (GROUND_LAYER - camera_pos.z) * TileSize : 0;
		view.view_scroll_x = (camera_pos.x * TileSize) + (TileSize / 2) - offset + offset_x - static_cast<int>(viewport_width * zoom / 2.0f);
		view.view_scroll_y = (camera_pos.y * TileSize) + (TileSize / 2) - offset + offset_y - static_cast<int>(viewport_height * zoom / 2.0f);

		// Matching RME's projection (width * zoom x height * zoom)
		view.projectionMatrix = glm::ortho(0.0f, static_cast<float>(viewport_width) * zoom, static_cast<float>(viewport_height) * zoom, 0.0f, -1.0f, 1.0f);
		view.viewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.375f, 0.375f, 0.0f));

		DrawingOptions options;
		options.SetIngame();
		options.show_lights = lighting_enabled;
		options.ambient_light_level = static_cast<float>(ambient_light) / 255.0f;
		options.light_intensity = light_intensity;
		// Explicitly set global light color to white (daylight) to avoid black multiplication
		options.global_light_color = wxColor(255, 255, 255);

		// Initialize GL state
		glViewport(viewport_x, viewport_y, viewport_width, viewport_height);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		primitive_renderer->setProjectionMatrix(view.projectionMatrix);
		light_buffer->Clear();
		if (creature_name_drawer) {
			creature_name_drawer->clear(); // Clear old labels
		}

		// Render floors from bottom to top
		for (int z = last_visible; z >= 0; z--) {
			float alpha = floor_opacity[z];
			if (alpha <= 0.001f) {
				continue;
			}

			// Sync viewport and start batch for this floor
			sprite_batch->begin(view.projectionMatrix);
			sprite_batch->setGlobalTint(1.0f, 1.0f, 1.0f, alpha);

			// Pre-calculate view offsets for this floor
			int floor_offset = (z <= GROUND_LAYER)
				? (GROUND_LAYER - z) * TileSize
				: TileSize * (view.floor - z);
			int camera_offset = (camera_pos.z <= GROUND_LAYER)
				? (GROUND_LAYER - camera_pos.z) * TileSize
				: 0;
			// offset_diff accounts for the diagonal shift between the camera floor and this floor
			int offset_diff = floor_offset - camera_offset;

			// Dynamic viewport culling — adjusted per floor
			// Use EXTREMELY large margins to guarantee no tiles are ever culled
			// The camera floor (z == camera_pos.z) uses view_scroll directly
			// Other floors are shifted by floor_offset, so we need to expand bounds
			constexpr int margin = TileSize * 16; // 16 tiles margin = 512 pixels

			// For viewport bounds, we need to consider the camera floor's coordinate system
			// The camera floor uses view_scroll directly (floor_offset = camera_offset)
			// For other floors, tiles are drawn at different positions due to floor_offset
			// To ensure ALL visible tiles on ANY floor are rendered, we expand bounds by max possible offset
			int max_floor_offset = std::max(
				std::abs(floor_offset - camera_offset),
				TileSize * MAP_MAX_LAYER // Maximum possible floor offset
			);

			view.start_x = static_cast<int>(std::floor((view.view_scroll_x - margin - max_floor_offset) / static_cast<float>(TileSize)));
			view.start_y = static_cast<int>(std::floor((view.view_scroll_y - margin - max_floor_offset) / static_cast<float>(TileSize)));
			view.end_x = static_cast<int>(std::ceil((view.view_scroll_x + viewport_width * zoom + margin + max_floor_offset) / static_cast<float>(TileSize)));
			view.end_y = static_cast<int>(std::ceil((view.view_scroll_y + viewport_height * zoom + margin + max_floor_offset) / static_cast<float>(TileSize)));

			int base_draw_x = -view.view_scroll_x - floor_offset;
			int base_draw_y = -view.view_scroll_y - floor_offset;

			for (int x = view.start_x; x <= view.end_x; ++x) {
				for (int y = view.start_y; y <= view.end_y; ++y) {
					const Tile* tile = map.getTile(x, y, z);
					if (tile) {
						int draw_x = (x * TileSize) + base_draw_x;
						int draw_y = (y * TileSize) + base_draw_y;
						tile_renderer->DrawTile(*sprite_batch, *primitive_renderer, tile->location, view, options, 0, draw_x, draw_y);
						if (lighting_enabled) {
							tile_renderer->AddLight(tile->location, view, options, *light_buffer);
						}
						// Add names of creatures on this floor
						if (creature_name_drawer && z == camera_pos.z) {
							if (tile->creature) {
								creature_name_drawer->addLabel(tile->location->getPosition(), tile->creature->getName(), tile->creature.get());
							}
						}
					}
				}
			}
			sprite_batch->end(*g_gui.gfx.getAtlasManager());
		}

		// Draw Preview Character (Center Screen)
		// Only if on the camera Z floor (or always visible as "player"?) - Request says "center of the screen".
		// Assuming always drawn on top.
		{
			sprite_batch->begin(view.projectionMatrix);

			// Calculate center position in logical coordinates
			int center_x = static_cast<int>((viewport_width * zoom) / 2.0f);
			int center_y = static_cast<int>((viewport_height * zoom) / 2.0f);

			// Adjust for sprite size (assuming 32x32 centered)
			// BlitCreature usually draws top-left at (screenx, screeny) relative to the tile grid logic?
			// It draws at (screenx, screeny).
			// We want center of sprite at center of screen.
			// 1. Fetch Elevation of current logical tile (camera_pos)
			int elevation_offset = GetTileElevationOffset(map.getTile(camera_pos));

			// 2. Adjust for sprite size (assuming 32x32 centered)
			// BlitCreature usually draws top-left at (screenx, screeny) relative to the tile grid logic?
			// It draws at (screenx, screeny).
			// We want center of sprite at center of screen.
			int draw_x = center_x - 16;
			int draw_y = center_y - 16 - elevation_offset;

			// Adjust for "walking offset" from standard drawing logic?
			// Standard Tile drawing: x * 32 - scroll_x.
			// Center of screen X in map coords = scroll_x + width/2.
			// Since we added offset_x/y to the camera scroll logic in IngamePreviewCanvas::Render,
			// the character should stay exactly at the center of the screen
			// while the map moves smoothly under it.
			// So we DO NOT add offset_x/y to draw_x/y again.

			creature_drawer->BlitCreature(*sprite_batch, sprite_drawer.get(), draw_x, draw_y, preview_outfit, preview_direction, 255, 255, 255, 255, animation_phase);

			sprite_batch->end(*g_gui.gfx.getAtlasManager());
		}

		if (lighting_enabled && light_drawer) {
			// Ensure light options are fully initialized to avoid black screen from garbage values
			options.experimental_fog = false;
			options.global_light_color = wxColor(255, 255, 255); // Full light color
			light_drawer->draw(view, options.experimental_fog, *light_buffer, options.global_light_color, options.light_intensity, options.ambient_light_level);
		}

		// Draw Names
		if (creature_name_drawer && vg) {
			TextRenderer::BeginFrame(vg, viewport_width, viewport_height, 1.0f); // Ingame preview doesn't use scale factor yet

			// 1. Draw creatures on map
			creature_name_drawer->draw(vg, view);

			// 2. Draw our own label at precise center
			if (vg) {
				nvgSave(vg);
				float fontSize = 11.0f;
				nvgFontSize(vg, fontSize);
				nvgFontFace(vg, "sans");
				nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);

				// Total elevation offset was calculated above.
				// For the label to stay synced, we should probably fetch it again or store it.
				// Since we are at the center of the screen, we can just use screen-relative coords.
				float screenCenterX = (float)viewport_width / 2.0f;
				float screenCenterY = (float)viewport_height / 2.0f;

				// Fetch elevation again to be precise
				int elevation_offset = GetTileElevationOffset(map.getTile(camera_pos));

				float labelY = screenCenterY - (16.0f + static_cast<float>(elevation_offset)) / zoom - 2.0f;

				std::string name = preview_name;
				float textBounds[4];
				nvgTextBounds(vg, 0, 0, name.c_str(), nullptr, textBounds);
				float textWidth = textBounds[2] - textBounds[0];
				float textHeight = textBounds[3] - textBounds[1];

				float paddingX = 4.0f;
				float paddingY = 2.0f;

				nvgBeginPath(vg);
				nvgRoundedRect(vg, screenCenterX - textWidth / 2.0f - paddingX, labelY - textHeight - paddingY * 2.0f, textWidth + paddingX * 2.0f, textHeight + paddingY * 2.0f, 3.0f);
				nvgFillColor(vg, nvgRGBA(0, 0, 0, 160));
				nvgFill(vg);

				nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
				nvgText(vg, screenCenterX, labelY - paddingY, name.c_str(), nullptr);
				nvgRestore(vg);
			}
			TextRenderer::EndFrame(vg);
		}
	}

	int IngamePreviewRenderer::GetTileElevationOffset(const Tile* tile) const {
		int elevation_offset = 0;
		if (tile) {
			for (const Item* item : tile->items) {
				elevation_offset += item->getHeight();
			}
			if (tile->ground) {
				elevation_offset += tile->ground->getHeight();
			}
			if (elevation_offset > 24) {
				elevation_offset = 24;
			}
		}
		return elevation_offset;
	}

} // namespace IngamePreview
