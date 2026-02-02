#include "ui/tool_options_surface.h"
#include "app/main.h"
#include "app/settings.h"
#include "ui/gui.h"
#include "rendering/core/graphics.h"
#include "ui/artprovider.h"
#include "brushes/managers/brush_manager.h"
#include "brushes/brush.h"
#include "game/sprites.h"

// Bring in specific brushes for identification/selection
#include "brushes/border/optional_border_brush.h"
#include "brushes/door/door_brush.h"
#include "brushes/flag/flag_brush.h"

BEGIN_EVENT_TABLE(ToolOptionsSurface, wxControl)
EVT_PAINT(ToolOptionsSurface::OnPaint)
EVT_ERASE_BACKGROUND(ToolOptionsSurface::OnEraseBackground)
EVT_MOUSE_EVENTS(ToolOptionsSurface::OnMouse)
EVT_LEAVE_WINDOW(ToolOptionsSurface::OnLeave)
EVT_SIZE(ToolOptionsSurface::OnSize)
EVT_TIMER(wxID_ANY, ToolOptionsSurface::OnTimer)
END_EVENT_TABLE()

ToolOptionsSurface::ToolOptionsSurface(wxWindow* parent) : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxWANTS_CHARS) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_animTimer.SetOwner(this);
}

ToolOptionsSurface::~ToolOptionsSurface() {
}

wxSize ToolOptionsSurface::DoGetBestClientSize() const {
	// Calculate height based on current layout
	// Base padding
	int h = FromDIP(8);

	// Tools Section
	if (!tool_rects.empty()) {
		int max_y = 0;
		for (const auto& tr : tool_rects) {
			max_y = std::max(max_y, tr.rect.GetBottom());
		}
		h = max_y + FromDIP(SECTION_GAP);
	}

	// Sliders Section
	if (interactables.size_slider_rect.height > 0) {
		h += interactables.size_slider_rect.height + FromDIP(GRID_GAP);
	}
	if (interactables.thickness_slider_rect.height > 0) {
		h += interactables.thickness_slider_rect.height + FromDIP(SECTION_GAP);
	}

	// Checkboxes
	if (interactables.preview_check_rect.height > 0) {
		h += interactables.preview_check_rect.height + FromDIP(4);
	}
	if (interactables.lock_check_rect.height > 0) {
		h += interactables.lock_check_rect.height + FromDIP(4);
	}

	// Min width 200 DIP?
	return wxSize(FromDIP(240), h + FromDIP(8));
}

void ToolOptionsSurface::DoSetSizeHints(int minW, int minH, int maxW, int maxH, int incW, int incH) {
	wxControl::DoSetSizeHints(minW, minH, maxW, maxH, incW, incH);
}

void ToolOptionsSurface::RebuildLayout() {
	tool_rects.clear();
	interactables.size_slider_rect = wxRect();
	interactables.thickness_slider_rect = wxRect();
	interactables.preview_check_rect = wxRect();
	interactables.lock_check_rect = wxRect();

	if (current_type == TILESET_UNKNOWN) {
		return;
	}

	int x = FromDIP(4);
	int y = FromDIP(4);
	const int w = GetClientSize().GetWidth();
	const int icon_sz = FromDIP(ICON_SIZE_LG); // Defaulting to large for "Pro" look
	const int gap = FromDIP(GRID_GAP);

	// 1. Tools
	bool has_tools = (current_type == TILESET_TERRAIN || current_type == TILESET_COLLECTION);

	if (has_tools) {
		// Populate tool list based on BrushManager
		std::vector<Brush*> brushes;

		if (g_brush_manager.optional_brush) {
			brushes.push_back(g_brush_manager.optional_brush);
		}
		if (g_brush_manager.eraser) {
			brushes.push_back(g_brush_manager.eraser);
		}
		if (g_brush_manager.pz_brush) {
			brushes.push_back(g_brush_manager.pz_brush);
		}
		if (g_brush_manager.rook_brush) {
			brushes.push_back(g_brush_manager.rook_brush);
		}
		if (g_brush_manager.nolog_brush) {
			brushes.push_back(g_brush_manager.nolog_brush);
		}
		if (g_brush_manager.pvp_brush) {
			brushes.push_back(g_brush_manager.pvp_brush);
		}

		// Doors?
		if (g_brush_manager.normal_door_brush) {
			brushes.push_back(g_brush_manager.normal_door_brush);
		}
		if (g_brush_manager.locked_door_brush) {
			brushes.push_back(g_brush_manager.locked_door_brush);
		}
		if (g_brush_manager.magic_door_brush) {
			brushes.push_back(g_brush_manager.magic_door_brush);
		}
		if (g_brush_manager.quest_door_brush) {
			brushes.push_back(g_brush_manager.quest_door_brush);
		}
		if (g_brush_manager.hatch_door_brush) {
			brushes.push_back(g_brush_manager.hatch_door_brush);
		}
		if (g_brush_manager.window_door_brush) {
			brushes.push_back(g_brush_manager.window_door_brush);
		}
		if (g_brush_manager.archway_door_brush) {
			brushes.push_back(g_brush_manager.archway_door_brush);
		}

		// Layout grid
		int cur_x = x;
		for (Brush* b : brushes) {
			if (cur_x + icon_sz > w) {
				cur_x = x;
				y += icon_sz + gap;
			}

			ToolRect tr;
			tr.rect = wxRect(cur_x, y, icon_sz, icon_sz);
			tr.brush = b;
			tr.tooltip = b->getName(); // Or specific label
			tool_rects.push_back(tr);

			cur_x += icon_sz + gap;
		}
		if (!brushes.empty()) {
			y += icon_sz + FromDIP(SECTION_GAP);
		}
	}

	int slider_h = FromDIP(24);
	int slider_w = w - FromDIP(8);

	// 2. Size Slider
	bool show_size = (current_type != TILESET_UNKNOWN); // Most show size
	if (show_size) {
		interactables.size_slider_rect = wxRect(x, y, slider_w, slider_h);
		y += slider_h + gap;
	}

	// 3. Thickness Slider
	bool show_thickness = (current_type == TILESET_COLLECTION || current_type == TILESET_DOODAD);
	if (show_thickness) {
		interactables.thickness_slider_rect = wxRect(x, y, slider_w, slider_h);
		y += slider_h + gap;
	}

	// 4. Options
	y += FromDIP(8); // Extra spacer
	if (has_tools) { // Assume terrain
		interactables.preview_check_rect = wxRect(x, y, slider_w, FromDIP(20));
		y += FromDIP(24);
		interactables.lock_check_rect = wxRect(x, y, slider_w, FromDIP(20));
	}

	InvalidateBestSize();
}

void ToolOptionsSurface::OnPaint(wxPaintEvent& evt) {
	wxAutoBufferedPaintDC dc(this);
	PrepareDC(dc);

	// Background
	wxColour bg = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
	dc.SetBackground(wxBrush(bg));
	dc.Clear();

	// 1. Draw Tools
	for (const auto& tr : tool_rects) {
		DrawToolIcon(dc, tr);
	}

	// 2. Draw Sliders
	if (interactables.size_slider_rect.height > 0) {
		DrawSlider(dc, interactables.size_slider_rect, "Size", current_size, 1, 15, true);
	}
	if (interactables.thickness_slider_rect.height > 0) {
		// Doodad thickness logic usually 0-100% or similar? No, old code was just "Thickness".
		DrawSlider(dc, interactables.thickness_slider_rect, "Thinkness", current_thickness, 1, 100, true);
	}

	// 3. Checkboxes
	if (interactables.preview_check_rect.height > 0) {
		DrawCheckbox(dc, interactables.preview_check_rect, "Preview Border", show_preview, interactables.hover_preview);
	}
	if (interactables.lock_check_rect.height > 0) {
		DrawCheckbox(dc, interactables.lock_check_rect, "Lock Doors (Shift)", lock_doors, interactables.hover_lock);
	}

	// Debug Focus
	// if (HasFocus()) { wxPen p(*wxRED, 1); dc.SetPen(p); dc.SetBrush(*wxTRANSPARENT_BRUSH); dc.DrawRectangle(GetClientSize()); }
}

void ToolOptionsSurface::DrawToolIcon(wxDC& dc, const ToolRect& tr) {
	bool is_selected = (active_brush == tr.brush);
	bool is_hover = (hover_brush == tr.brush);

	wxRect r = tr.rect;

	// Background
	if (is_selected) {
		dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
		dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT).ChangeLightness(180)));
		dc.DrawRectangle(r);
	} else if (is_hover) {
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(wxColour(200, 200, 200, 100))); // Transparent grey
		dc.DrawRectangle(r);
	}

	// Draw Brush Sprite
	if (tr.brush) {
		Sprite* s = tr.brush->getSprite();
		if (!s && tr.brush->getLookID() != 0) {
			s = g_gui.gfx.getSprite(tr.brush->getLookID());
		}

		if (s) {
			// Center the sprite in the rect
			// Assuming 32x32 icon size for now, which matches SPRITE_SIZE_32x32 (32x32)
			int x_off = r.x + (r.width - 32) / 2;
			int y_off = r.y + (r.height - 32) / 2;
			s->DrawTo(&dc, SPRITE_SIZE_32x32, x_off, y_off);
		} else {
			// Fallback text/color if no sprite
			wxString label = tr.tooltip.Left(1);
			dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
			dc.DrawLabel(label, r, wxALIGN_CENTER);
		}
	}

	// Border
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	if (is_selected) {
		dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
	} else {
		dc.SetPen(wxPen(wxColour(100, 100, 100)));
	}
	dc.DrawRectangle(r);
}

void ToolOptionsSurface::DrawSlider(wxDC& dc, const wxRect& rect, const wxString& label, int value, int min, int max, bool active) {
	// Label
	dc.SetFont(GetFont());
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

	wxSize extent = dc.GetTextExtent(label);
	dc.DrawText(label, rect.GetLeft(), rect.GetTop() + (rect.height - extent.y) / 2);

	// Track
	int label_w = FromDIP(70);
	int track_x = rect.GetLeft() + label_w;
	int track_w = rect.width - label_w - FromDIP(40); // Room for value text
	int track_h = FromDIP(4);
	int track_y = rect.GetTop() + (rect.height - track_h) / 2;

	wxRect track_rect(track_x, track_y, track_w, track_h);

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(wxColour(200, 200, 200)));
	dc.DrawRectangle(track_rect);

	// Fill
	if (value > min && max > min) {
		float pct = (float)(value - min) / (float)(max - min);
		int fill_w = (int)(track_w * pct);
		wxRect fill_rect(track_x, track_y, fill_w, track_h);
		dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
		dc.DrawRectangle(fill_rect);

		// Thumb
		dc.SetBrush(wxBrush(wxColour(50, 50, 50)));
		dc.DrawCircle(track_x + fill_w, track_y + track_h / 2, FromDIP(5));
	}

	// Value Text
	wxString val_str = wxString::Format("%d", value);
	dc.DrawText(val_str, track_x + track_w + FromDIP(8), rect.GetTop() + (rect.height - extent.y) / 2);
}

void ToolOptionsSurface::DrawCheckbox(wxDC& dc, const wxRect& rect, const wxString& label, bool value, bool hover) {
	// Box
	int box_sz = FromDIP(14);
	int box_y = rect.GetTop() + (rect.height - box_sz) / 2;
	wxRect box(rect.GetLeft(), box_y, box_sz, box_sz);

	if (hover) {
		dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)));
	} else {
		dc.SetPen(wxPen(wxColour(150, 150, 150)));
	}

	dc.SetBrush(value ? wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)) : *wxWHITE_BRUSH);
	dc.DrawRectangle(box);

	// Checkmark (simple)
	if (value) {
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawLine(box.GetLeft() + 3, box.GetTop() + 7, box.GetLeft() + 6, box.GetTop() + 10);
		dc.DrawLine(box.GetLeft() + 6, box.GetTop() + 10, box.GetRight() - 3, box.GetTop() + 4);
	}

	// Label
	dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
	dc.DrawText(label, box.GetRight() + FromDIP(8), box_y - 1);
}

void ToolOptionsSurface::OnEraseBackground(wxEraseEvent& evt) {
	// No-op
}

void ToolOptionsSurface::OnMouse(wxMouseEvent& evt) {
	m_hoverPos = evt.GetPosition();

	// Hit testing
	Brush* prev_hover = hover_brush;
	hover_brush = nullptr;

	bool prev_preview = interactables.hover_preview;
	bool prev_lock = interactables.hover_lock;
	interactables.hover_preview = false;
	interactables.hover_lock = false;

	// Tools
	for (const auto& tr : tool_rects) {
		if (tr.rect.Contains(m_hoverPos)) {
			hover_brush = tr.brush;
			// Tooltip handling could go here (delayed)
			break;
		}
	}

	// Sliders Interaction
	if (evt.LeftDown()) {
		if (interactables.size_slider_rect.Contains(m_hoverPos)) {
			interactables.dragging_size = true;
			CaptureMouse();
		} else if (interactables.thickness_slider_rect.Contains(m_hoverPos)) {
			interactables.dragging_thickness = true;
			CaptureMouse();
		} else if (interactables.preview_check_rect.Contains(m_hoverPos)) {
			show_preview = !show_preview;
			g_settings.setInteger(Config::SHOW_AUTOBORDER_PREVIEW, show_preview);
			Refresh();
		} else if (interactables.lock_check_rect.Contains(m_hoverPos)) {
			lock_doors = !lock_doors;
			g_settings.setInteger(Config::DRAW_LOCKED_DOOR, lock_doors);
			g_brush_manager.SetDoorLocked(lock_doors);
			Refresh();
		} else if (hover_brush) {
			SelectBrush(hover_brush);
		}
	}

	if (evt.Dragging() && evt.LeftIsDown()) {
		if (interactables.dragging_size) {
			// Calculate value
			// In a real impl, refactor the math to a helper
			int label_w = FromDIP(70);
			int track_x = interactables.size_slider_rect.GetLeft() + label_w;
			int track_w = interactables.size_slider_rect.width - label_w - FromDIP(40);

			int rel_x = m_hoverPos.x - track_x;
			float pct = (float)rel_x / (float)track_w;
			if (pct < 0) {
				pct = 0;
			}
			if (pct > 1) {
				pct = 1;
			}

			int min = 1, max = 15;
			int val = min + (int)(pct * (max - min));
			if (val != current_size) {
				current_size = val;
				g_brush_manager.SetBrushSize(current_size); // Assuming square for now
				Refresh();
			}
		}
		// Thickness similar...
	}

	if (evt.LeftUp()) {
		if (GetCapture() == this) {
			ReleaseMouse();
		}
		interactables.dragging_size = false;
		interactables.dragging_thickness = false;
	}

	// Hover states for checkboxes
	if (interactables.preview_check_rect.Contains(m_hoverPos)) {
		interactables.hover_preview = true;
	}
	if (interactables.lock_check_rect.Contains(m_hoverPos)) {
		interactables.hover_lock = true;
	}

	if (prev_hover != hover_brush || prev_preview != interactables.hover_preview || prev_lock != interactables.hover_lock) {
		Refresh();
	}
}

void ToolOptionsSurface::OnLeave(wxMouseEvent& evt) {
	hover_brush = nullptr;
	Refresh();
}

void ToolOptionsSurface::OnTimer(wxTimerEvent& evt) {
	// Anim logic if needed
}

void ToolOptionsSurface::OnSize(wxSizeEvent& evt) {
	RebuildLayout();
	Refresh();
	evt.Skip();
}

void ToolOptionsSurface::SetPaletteType(PaletteType type) {
	if (current_type == type) {
		return;
	}
	current_type = type;
	RebuildLayout();
	Refresh();
}

void ToolOptionsSurface::UpdateBrushSize(BrushShape shape, int size) {
	if (current_size != size) {
		current_size = size;
		Refresh();
	}
}

void ToolOptionsSurface::ReloadSettings() {
	show_preview = g_settings.getInteger(Config::SHOW_AUTOBORDER_PREVIEW);
	lock_doors = g_settings.getInteger(Config::DRAW_LOCKED_DOOR);
	RebuildLayout();
	Refresh();
}

void ToolOptionsSurface::SelectBrush(Brush* brush) {
	if (active_brush == brush) {
		return;
	}
	active_brush = brush;
	g_brush_manager.SelectBrush(brush);
	Refresh();
}
