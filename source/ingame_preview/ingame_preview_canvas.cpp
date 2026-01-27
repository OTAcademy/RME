#include "ingame_preview/ingame_preview_canvas.h"
#include "ingame_preview/ingame_preview_renderer.h"
#include "editor/editor.h"
#include "ui/gui.h"
#include "rendering/map_drawer.h"
#include "rendering/ui/map_display.h"

namespace IngamePreview {

	BEGIN_EVENT_TABLE(IngamePreviewCanvas, wxGLCanvas)
	EVT_PAINT(IngamePreviewCanvas::OnPaint)
	EVT_SIZE(IngamePreviewCanvas::OnSize)
	EVT_MOTION(IngamePreviewCanvas::OnMouseMove)
	EVT_ERASE_BACKGROUND(IngamePreviewCanvas::OnEraseBackground)
	END_EVENT_TABLE()

	// Re-use core profile attributes from MapCanvas logic
	static wxGLAttributes& GetPreviewGLAttributes() {
		static wxGLAttributes vAttrs = []() {
			wxGLAttributes a;
			a.PlatformDefaults().Defaults().RGBA().DoubleBuffer().Depth(24).Stencil(8).EndList();
			return a;
		}();
		return vAttrs;
	}

	IngamePreviewCanvas::IngamePreviewCanvas(wxWindow* parent) :
		wxGLCanvas(parent, GetPreviewGLAttributes(), wxID_ANY, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS),
		camera_pos(0, 0, GROUND_LAYER),
		zoom(1.0f),
		lighting_enabled(true),
		ambient_light(255),
		light_intensity(1.0f),
		viewport_width_tiles(15),
		viewport_height_tiles(11) {
	}

	IngamePreviewCanvas::~IngamePreviewCanvas() = default;

	void IngamePreviewCanvas::OnPaint(wxPaintEvent& event) {
		// Render(); // Remove auto-render on paint, triggered by timer now
	}

	void IngamePreviewCanvas::OnSize(wxSizeEvent& event) {
		Refresh();
	}

	void IngamePreviewCanvas::OnMouseMove(wxMouseEvent& event) {
		event.Skip();
	}

	void IngamePreviewCanvas::SetCameraPosition(const Position& pos) {
		if (camera_pos != pos) {
			camera_pos = pos;
			Refresh();
		}
	}

	void IngamePreviewCanvas::SetZoom(float z) {
		if (zoom != z) {
			zoom = z;
			Refresh();
		}
	}

	void IngamePreviewCanvas::SetLightingEnabled(bool enabled) {
		if (lighting_enabled != enabled) {
			lighting_enabled = enabled;
			Refresh();
		}
	}

	void IngamePreviewCanvas::SetAmbientLight(uint8_t ambient) {
		if (ambient_light != ambient) {
			ambient_light = ambient;
			Refresh();
		}
	}

	void IngamePreviewCanvas::SetLightIntensity(float intensity) {
		if (light_intensity != intensity) {
			light_intensity = intensity;
			Refresh();
		}
	}

	void IngamePreviewCanvas::SetViewportSize(int w, int h) {
		if (viewport_width_tiles != w || viewport_height_tiles != h) {
			viewport_width_tiles = w;
			viewport_height_tiles = h;
			Refresh();
		}
	}

	void IngamePreviewCanvas::GetViewportSize(int& w, int& h) const {
		w = viewport_width_tiles;
		h = viewport_height_tiles;
	}

	void IngamePreviewCanvas::Render(Editor* current_editor) {
		if (!IsShownOnScreen() || !current_editor) {
			return;
		}

		SetCurrent(*g_gui.GetGLContext(this));

		if (!renderer) {
			MapTab* tab = g_gui.GetCurrentMapTab();
			if (tab && tab->GetCanvas() && tab->GetCanvas()->drawer) {
				renderer = std::make_unique<IngamePreviewRenderer>(tab->GetCanvas()->drawer->getTileRenderer());
			} else {
				return;
			}
		}

		wxSize size = GetClientSize();

		float content_w = viewport_width_tiles * 32.0f;
		float content_h = viewport_height_tiles * 32.0f;

		// Avoid division by zero or invalid sizes
		if (content_w <= 0.1f || content_h <= 0.1f || size.x <= 1 || size.y <= 1) {
			return;
		}

		float scale_x = (float)size.x / content_w;
		float scale_y = (float)size.y / content_h;
		float scale = std::min(scale_x, scale_y);

		int view_w = (int)(content_w * scale);
		int view_h = (int)(content_h * scale);
		int view_x = (size.x - view_w) / 2;
		int view_y = (size.y - view_h) / 2;

		float calculated_zoom = 1.0f / scale;

		renderer->SetLightIntensity(light_intensity);
		renderer->Render(current_editor->map, view_x, view_y, view_w, view_h, camera_pos, calculated_zoom, lighting_enabled, ambient_light);

		SwapBuffers();
	}

} // namespace IngamePreview
