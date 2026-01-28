#include "ingame_preview/ingame_preview_canvas.h"
#include "ingame_preview/ingame_preview_renderer.h"
#include "editor/editor.h"
#include "ui/gui.h"
#include "rendering/map_drawer.h"
#include "rendering/ui/map_display.h"

namespace IngamePreview {

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
		last_tile_renderer(nullptr),
		camera_pos(0, 0, GROUND_LAYER),
		zoom(1.0f),
		lighting_enabled(true),
		ambient_light(255),
		light_intensity(1.0f),
		viewport_width_tiles(15),
		viewport_height_tiles(11) {

		// Bind Events
		Bind(wxEVT_PAINT, &IngamePreviewCanvas::OnPaint, this);
		Bind(wxEVT_SIZE, &IngamePreviewCanvas::OnSize, this);
		Bind(wxEVT_MOTION, &IngamePreviewCanvas::OnMouseMove, this);
		Bind(wxEVT_ERASE_BACKGROUND, &IngamePreviewCanvas::OnEraseBackground, this);
	}

	IngamePreviewCanvas::~IngamePreviewCanvas() = default;

	void IngamePreviewCanvas::OnPaint(wxPaintEvent& event) {
		// Validating the paint event prevents infinite paint loops on some platforms
		wxPaintDC dc(this);
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

		MapTab* tab = g_gui.GetCurrentMapTab();
		const void* current_tile_renderer = nullptr;

		if (tab && tab->GetCanvas() && tab->GetCanvas()->drawer) {
			current_tile_renderer = tab->GetCanvas()->drawer->getTileRenderer();
		}

		// Check if renderer is stale or missing
		if (!renderer || last_tile_renderer != current_tile_renderer) {
			if (current_tile_renderer) {
				// We need to cast back to TileRenderer* to create the preview renderer
				// Implementation note: The header uses void* to avoid includes, but here we see map_drawer.h
				// so we know what getTileRenderer returns.
				renderer = std::make_unique<IngamePreviewRenderer>(
					const_cast<TileRenderer*>(static_cast<const TileRenderer*>(current_tile_renderer))
				);
				last_tile_renderer = current_tile_renderer;
			} else {
				// No valid tile renderer available
				renderer.reset();
				last_tile_renderer = nullptr;
				return;
			}
		}

		if (!renderer) {
			return;
		}

		wxSize size = GetClientSize();

		float content_w = viewport_width_tiles * 32.0f;
		float content_h = viewport_height_tiles * 32.0f;

		// Avoid division by zero or invalid sizes
		if (content_w <= 0.1f || content_h <= 0.1f || size.x <= 1 || size.y <= 1) {
			return;
		}

		float scale_x = static_cast<float>(size.x) / content_w;
		float scale_y = static_cast<float>(size.y) / content_h;
		float scale = std::min(scale_x, scale_y);

		int view_w = static_cast<int>(content_w * scale);
		int view_h = static_cast<int>(content_h * scale);
		int view_x = (size.x - view_w) / 2;
		int view_y = (size.y - view_h) / 2;

		float calculated_zoom = 1.0f / scale;

		renderer->SetLightIntensity(light_intensity);
		renderer->Render(current_editor->map, view_x, view_y, view_w, view_h, camera_pos, calculated_zoom, lighting_enabled, ambient_light);

		SwapBuffers();
	}

} // namespace IngamePreview
