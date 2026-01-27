#ifndef RME_INGAME_PREVIEW_CANVAS_H_
#define RME_INGAME_PREVIEW_CANVAS_H_

#include "app/main.h"
#include "map/position.h"
#include <memory>

class Editor;

namespace IngamePreview {

	class IngamePreviewRenderer;

	class IngamePreviewCanvas : public wxGLCanvas {
	public:
		IngamePreviewCanvas(wxWindow* parent);
		~IngamePreviewCanvas();

		void OnPaint(wxPaintEvent& event);
		void OnSize(wxSizeEvent& event);
		void OnMouseMove(wxMouseEvent& event);
		void OnEraseBackground(wxEraseEvent& event) { }

		void SetCameraPosition(const Position& pos);
		void SetZoom(float z);
		void SetLightingEnabled(bool enabled);
		void SetAmbientLight(uint8_t ambient);
		void SetLightIntensity(float intensity);
		void SetViewportSize(int w, int h);
		void GetViewportSize(int& w, int& h) const;

		void Render(Editor* current_editor);

	private:
		std::unique_ptr<IngamePreviewRenderer> renderer;
		const void* last_tile_renderer; // Using void* to avoid forward declaration issues if TileRenderer isn't fully known, but forward decl is better.
		// Actually, let's look at lines 12. IngamePreviewRenderer is forward declared.
		// TileRenderer is in map_drawer implementation details usually.
		// Let's check if we can simply use const void* or if we need forward declaration.
		// IngamePreviewCanvas.cpp includes "rendering/map_drawer.h" which usually has TileRenderer.
		// But in header we might not have it.
		// Let's use const void* for safety or forward declare class TileRenderer;

		Position camera_pos;
		float zoom;
		bool lighting_enabled;
		uint8_t ambient_light;
		float light_intensity;

		int viewport_width_tiles;
		int viewport_height_tiles;
	};

} // namespace IngamePreview

#endif // RME_INGAME_PREVIEW_CANVAS_H_
