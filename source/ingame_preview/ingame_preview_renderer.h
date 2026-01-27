#ifndef RME_INGAME_PREVIEW_RENDERER_H_
#define RME_INGAME_PREVIEW_RENDERER_H_

#include "app/main.h"
#include "map/position.h"
#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include <memory>
#include <map>
#include <chrono>

class BaseMap;
class TileRenderer;
class SpriteBatch;
class PrimitiveRenderer;
class LightDrawer;
struct LightBuffer;

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
		void Render(const BaseMap& map, int viewport_x, int viewport_y, int viewport_width, int viewport_height, const Position& camera_pos, float zoom, bool lighting_enabled, uint8_t ambient_light);

		void SetLightIntensity(float intensity) {
			light_intensity = intensity;
		}

	private:
		TileRenderer* tile_renderer;
		std::unique_ptr<FloorVisibilityCalculator> floor_calculator;

		float light_intensity = 1.0f;

		// Smooth fading state
		std::map<int, float> floor_opacity;
		std::chrono::steady_clock::time_point last_time;

		// Internal rendering resources (could be shared or managed)
		std::unique_ptr<SpriteBatch> sprite_batch;
		std::unique_ptr<PrimitiveRenderer> primitive_renderer;
		std::unique_ptr<LightBuffer> light_buffer;
		std::shared_ptr<LightDrawer> light_drawer;

		void UpdateOpacity(double dt, int first_visible, int last_visible);
	};

} // namespace IngamePreview

#endif // RME_INGAME_PREVIEW_RENDERER_H_
