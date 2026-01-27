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

		Position camera_pos;
		float zoom;
		bool lighting_enabled;
		uint8_t ambient_light;
		float light_intensity;

		int viewport_width_tiles;
		int viewport_height_tiles;

		DECLARE_EVENT_TABLE()
	};

} // namespace IngamePreview

#endif // RME_INGAME_PREVIEW_CANVAS_H_
