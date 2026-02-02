#ifndef RME_INGAME_PREVIEW_RENDERER_H_
#define RME_INGAME_PREVIEW_RENDERER_H_

#include "app/main.h"
#include "map/position.h"
#include "game/creature.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include <memory>
#include <map>
#include <chrono>
#include <string>

class Tile;
class BaseMap;
class TileRenderer;
class SpriteBatch;
class PrimitiveRenderer;
class LightDrawer;
struct LightBuffer;
class CreatureDrawer;
class CreatureNameDrawer;
class SpriteDrawer;
struct Outfit;

namespace IngamePreview {

	class FloorVisibilityCalculator;

	/**
	 * High-level renderer for the in-game preview window.
	 */
	class IngamePreviewRenderer {
	public:
		IngamePreviewRenderer(TileRenderer* tile_renderer);
		~IngamePreviewRenderer();

		/**
		 * Render the preview.
		 */
		void Render(const BaseMap& map, int viewport_x, int viewport_y, int viewport_width, int viewport_height, const Position& camera_pos, float zoom, bool lighting_enabled, uint8_t ambient_light, const Outfit& preview_outfit, Direction preview_direction, int animation_phase, int offset_x, int offset_y);

		void SetLightIntensity(float intensity) {
			light_intensity = intensity;
		}

		void SetName(const std::string& name) {
			preview_name = name;
		}

	private:
		TileRenderer* tile_renderer;
		std::unique_ptr<FloorVisibilityCalculator> floor_calculator;

		float light_intensity = 1.0f;
		std::string preview_name = "You";

		// Smooth fading state
		std::map<int, float> floor_opacity;
		std::chrono::steady_clock::time_point last_time;

		// Internal rendering resources (could be shared or managed)
		std::unique_ptr<SpriteBatch> sprite_batch;
		std::unique_ptr<PrimitiveRenderer> primitive_renderer;
		std::unique_ptr<LightBuffer> light_buffer;
		std::shared_ptr<LightDrawer> light_drawer;

		// Drawers
		std::unique_ptr<CreatureDrawer> creature_drawer;
		std::unique_ptr<CreatureNameDrawer> creature_name_drawer;
		std::unique_ptr<SpriteDrawer> sprite_drawer;

		void UpdateOpacity(double dt, int first_visible, int last_visible);
		int GetTileElevationOffset(const Tile* tile) const;
	};

} // namespace IngamePreview

#endif // RME_INGAME_PREVIEW_RENDERER_H_
