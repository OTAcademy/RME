#include "ingame_preview/ingame_preview_canvas.h"
#include "ingame_preview/ingame_preview_renderer.h"
#include "ingame_preview/ingame_preview_window.h"
#include "editor/editor.h"
#include "ui/gui.h"
#include "rendering/map_drawer.h"
#include "rendering/ui/map_display.h"
#include "rendering/core/text_renderer.h"
#include <spdlog/spdlog.h>
#include <glad/glad.h>
#include <nanovg.h>
#include <nanovg_gl.h>

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
		viewport_height_tiles(11),
		preview_direction(SOUTH),
		is_walking(false),
		walk_start_time(0),
		walk_duration(200),
		walk_direction(SOUTH),
		next_walk_direction(SOUTH),
		walk_source_pos(0, 0, GROUND_LAYER),
		walk_offset_x(0),
		walk_offset_y(0),
		animation_phase(0),
		speed(220),
		last_step_time(0),
		walk_lock_timer(0),
		animation_timer(this) {

		preview_outfit.lookType = 128;

		// Bind Events
		Bind(wxEVT_PAINT, &IngamePreviewCanvas::OnPaint, this);
		Bind(wxEVT_SIZE, &IngamePreviewCanvas::OnSize, this);
		Bind(wxEVT_MOTION, &IngamePreviewCanvas::OnMouseMove, this);
		Bind(wxEVT_KEY_DOWN, &IngamePreviewCanvas::OnKeyDown, this);
		Bind(wxEVT_TIMER, &IngamePreviewCanvas::OnTimer, this);
		Bind(wxEVT_ERASE_BACKGROUND, &IngamePreviewCanvas::OnEraseBackground, this);

		animation_timer.Start(16); // ~60 FPS update
	}

	IngamePreviewCanvas::~IngamePreviewCanvas() = default;

	void IngamePreviewCanvas::OnPaint(wxPaintEvent& event) {
		// Validating the paint event prevents infinite paint loops on some platforms
		wxPaintDC dc(this); // validates the paint event

		if (auto* tab = g_gui.GetCurrentMapTab()) {
			Render(tab->GetEditor());
		}
	}

	void IngamePreviewCanvas::OnSize(wxSizeEvent& event) {
		Refresh();
	}

	void IngamePreviewCanvas::OnMouseMove(wxMouseEvent& event) {
		event.Skip();
	}

	void IngamePreviewCanvas::OnKeyDown(wxKeyEvent& event) {
		int key = event.GetKeyCode();
		bool ctrl = event.ControlDown();
		bool shift = event.ShiftDown();

		// Floor Change (Shift + Up/Down)
		// Floor Change (Shift + Up/Down)
		if (shift && (key == WXK_UP || key == WXK_DOWN)) {
			Position pos = camera_pos;
			if (key == WXK_UP) {
				pos.z = std::max(0, pos.z - 1);
			} else { // WXK_DOWN
				pos.z = std::min(MAP_MAX_LAYER, pos.z + 1);
			}

			SetCameraPosition(pos);
			if (auto* parent = dynamic_cast<IngamePreviewWindow*>(GetParent())) {
				parent->SetFollowSelection(false);
			}
			return;
		}

		Direction dir = SOUTH;
		bool directional = false;
		if (key == WXK_UP) {
			dir = NORTH;
			directional = true;
		} else if (key == WXK_DOWN) {
			dir = SOUTH;
			directional = true;
		} else if (key == WXK_LEFT) {
			dir = WEST;
			directional = true;
		} else if (key == WXK_RIGHT) {
			dir = EAST;
			directional = true;
		}

		if (directional) {
			if (auto* parent = dynamic_cast<IngamePreviewWindow*>(GetParent())) {
				parent->SetFollowSelection(false);
			}

			if (ctrl) {
				Turn(dir);
			} else {
				BufferWalk(dir);
			}
		} else {
			event.Skip();
		}
	}

	void IngamePreviewCanvas::Turn(Direction dir) {
		if (is_walking) {
			return;
		}
		preview_direction = dir;
		Refresh();
	}

	void IngamePreviewCanvas::BufferWalk(Direction dir) {
		if (walk_queue.empty()) {
			walk_queue.push_back(dir);
			UpdateWalk(); // Try to start immediately
		} else if (walk_queue.size() < 2) {
			if (walk_queue.back() != dir) {
				walk_queue.push_back(dir);
			}
		}
	}

	bool IngamePreviewCanvas::CanWalk() {
		// Collision checking is done in StartWalk now
		return true;
	}

	int IngamePreviewCanvas::GetStepDuration(uint16_t ground_speed) {
		if (speed <= 0) {
			speed = 220;
		}
		if (ground_speed <= 0) {
			ground_speed = 100;
		}

		int interval = (1000 * ground_speed) / speed;
		return std::max(50, std::min(interval, 1000));
	}

	void IngamePreviewCanvas::OnTimer(wxTimerEvent& event) {
		UpdateWalk();
		Refresh();
	}

	void IngamePreviewCanvas::StartWalk(Direction dir) {
		if (is_walking) {
			return;
		}

		// 1. Calculate Target
		Position target_pos = camera_pos;
		if (dir == NORTH) {
			target_pos.y--;
		} else if (dir == SOUTH) {
			target_pos.y++;
		} else if (dir == WEST) {
			target_pos.x--;
		} else if (dir == EAST) {
			target_pos.x++;
		}

		// 2. Collision Check
		uint16_t ground_speed = 100;
		if (auto* tab = g_gui.GetCurrentMapTab()) {
			if (auto* editor = tab->GetEditor()) {
				const Tile* tile = editor->map.getTile(target_pos);
				if (!tile || tile->isBlocking()) {
					walk_queue.clear();
					return;
				}
				if (const Item* ground = tile->ground) {
					ground_speed = g_items[ground->getID()].way_speed;
				}
			}
		}

		// 3. Initiate Walk
		is_walking = true;
		walk_direction = dir;
		preview_direction = dir;
		walk_source_pos = camera_pos;

		walk_start_time = wxGetLocalTimeMillis().GetValue();
		walk_duration = GetStepDuration(ground_speed);

		// Initialize walk offsets to compensate for immediate logical move
		walk_offset_x = 0;
		walk_offset_y = 0;
		if (dir == NORTH) {
			walk_offset_y = 32;
		} else if (dir == SOUTH) {
			walk_offset_y = -32;
		} else if (dir == WEST) {
			walk_offset_x = 32;
		} else if (dir == EAST) {
			walk_offset_x = -32;
		}

		// Move logically immediately
		camera_pos = target_pos;
	}

	void IngamePreviewCanvas::UpdateWalk() {
		if (!is_walking && !walk_queue.empty()) {
			Direction next = walk_queue.front();
			walk_queue.pop_front();
			StartWalk(next);
		}

		if (!is_walking) {
			if (animation_phase != 0) {
				spdlog::debug("UpdateWalk: Resetting animation_phase from {} to 0 (not walking)", animation_phase);
				animation_phase = 0;
			}
			return;
		}

		long long now = wxGetLocalTimeMillis().GetValue();
		float progress = static_cast<float>(now - walk_start_time) / walk_duration;

		if (progress >= 1.0f) {
			is_walking = false;
			walk_offset_x = 0;
			walk_offset_y = 0;
			animation_phase = 0;
			last_step_time = now;

			// Try next step
			if (!walk_queue.empty()) {
				UpdateWalk();
			}
			return;
		}

		// Interpolation
		int offset_pixels = static_cast<int>(32.0f * (1.0f - progress));
		walk_offset_x = 0;
		walk_offset_y = 0;

		if (walk_direction == NORTH) {
			walk_offset_y = offset_pixels;
		} else if (walk_direction == SOUTH) {
			walk_offset_y = -offset_pixels;
		} else if (walk_direction == WEST) {
			walk_offset_x = offset_pixels;
		} else if (walk_direction == EAST) {
			walk_offset_x = -offset_pixels;
		}

		// 1-2 animation loop (Tibia standard walk is Frame 1 and Frame 2)
		// We use 4 as a multiplier to make it feel a bit faster/standard
		animation_phase = 1 + (static_cast<int>(progress * 4.0f) % 2);
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
		
		// DEBUG: Check if animation_phase is non-zero when not walking
		if (animation_phase != 0 && !is_walking) {
			spdlog::warn("Render: animation_phase={} but is_walking=false! This shouldn't happen.", animation_phase);
		}

		SetCurrent(*g_gui.GetGLContext(this));

		if (!m_nvg) {
			if (!gladLoadGL()) {
				spdlog::error("IngamePreviewCanvas: Failed to initialize GLAD");
			}
			m_nvg.reset(nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES));
			if (m_nvg) {
				TextRenderer::LoadFont(m_nvg.get());
			}
		}

		MapTab* tab = g_gui.GetCurrentMapTab();
		const void* current_tile_renderer = nullptr;

		if (tab && tab->GetCanvas() && tab->GetCanvas()->drawer) {
			current_tile_renderer = tab->GetCanvas()->drawer->getTileRenderer();
		}

		if (!renderer || last_tile_renderer != current_tile_renderer) {
			if (current_tile_renderer) {
				renderer = std::make_unique<IngamePreviewRenderer>(
					const_cast<TileRenderer*>(static_cast<const TileRenderer*>(current_tile_renderer))
				);
				last_tile_renderer = current_tile_renderer;
			} else {
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

		NVGcontext* vg = m_nvg.get();
		renderer->SetLightIntensity(light_intensity);
		renderer->SetName(preview_name_str);
		renderer->Render(vg, current_editor->map, view_x, view_y, view_w, view_h, camera_pos, calculated_zoom, lighting_enabled, ambient_light, preview_outfit, preview_direction, animation_phase, walk_offset_x, walk_offset_y);

		SwapBuffers();
	}

} // namespace IngamePreview
