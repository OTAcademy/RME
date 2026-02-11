#include "ui/tool_options_surface.h"
#include "app/main.h"
#include "app/settings.h"
#include "ui/gui.h"
#include "ui/theme.h"
#include "rendering/core/graphics.h"
#include "ui/artprovider.h"
#include "brushes/managers/brush_manager.h"
#include "brushes/brush.h"
#include "game/sprites.h"
#include <format>
#include <algorithm>

// Bring in specific brushes for identification/selection
#include "brushes/border/optional_border_brush.h"
#include "brushes/door/door_brush.h"
#include "brushes/flag/flag_brush.h"

ToolOptionsSurface::ToolOptionsSurface(wxWindow* parent) : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxWANTS_CHARS) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	m_animTimer.SetOwner(this);

	Bind(wxEVT_PAINT, &ToolOptionsSurface::OnPaint, this);
	Bind(wxEVT_ERASE_BACKGROUND, &ToolOptionsSurface::OnEraseBackground, this);
	Bind(wxEVT_LEFT_DOWN, &ToolOptionsSurface::OnMouse, this);
	Bind(wxEVT_LEFT_DCLICK, &ToolOptionsSurface::OnMouse, this);
	Bind(wxEVT_LEFT_UP, &ToolOptionsSurface::OnMouse, this);
	Bind(wxEVT_MOTION, &ToolOptionsSurface::OnMouse, this);
	Bind(wxEVT_LEAVE_WINDOW, &ToolOptionsSurface::OnLeave, this);
	Bind(wxEVT_SIZE, &ToolOptionsSurface::OnSize, this);
	Bind(wxEVT_TIMER, &ToolOptionsSurface::OnTimer, this);
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
	wxColour bg = Theme::Get(Theme::Role::Surface);
	dc.SetBackground(wxBrush(bg));
	dc.Clear();

	// 1. Draw Tools
	for (const auto& tr : tool_rects) {
		DrawToolIcon(dc, tr);
	}

	// 2. Draw Sliders
	if (interactables.size_slider_rect.height > 0) {
		DrawSlider(dc, interactables.size_slider_rect, "Size", current_size, MIN_BRUSH_SIZE, MAX_BRUSH_SIZE, true);
	}
	if (interactables.thickness_slider_rect.height > 0) {
		// Doodad thickness logic usually 0-100% or similar? No, old code was just "Thickness".
		DrawSlider(dc, interactables.thickness_slider_rect, "Thickness", current_thickness, MIN_BRUSH_THICKNESS, MAX_BRUSH_THICKNESS, true);
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
		dc.SetPen(wxPen(Theme::Get(Theme::Role::Accent)));
		dc.SetBrush(wxBrush(Theme::Get(Theme::Role::Selected)));
		dc.DrawRectangle(r);
	} else if (is_hover) {
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.SetBrush(wxBrush(wxColour(255, 255, 255, 30))); // Transparent white
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
			dc.SetTextForeground(Theme::Get(Theme::Role::Text));
			dc.DrawLabel(label, r, wxALIGN_CENTER);
		}
	}

	// Border
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	if (is_selected) {
		dc.SetPen(wxPen(Theme::Get(Theme::Role::Accent)));
	} else {
		dc.SetPen(wxPen(Theme::Get(Theme::Role::Border)));
	}
	dc.DrawRectangle(r);
}

void ToolOptionsSurface::DrawSlider(wxDC& dc, const wxRect& rect, const wxString& label, int value, int min, int max, bool active) {
	// Label
	dc.SetFont(GetFont());
	dc.SetTextForeground(Theme::Get(Theme::Role::Text));

	wxSize extent = dc.GetTextExtent(label);
	dc.DrawText(label, rect.GetLeft(), rect.GetTop() + (rect.height - extent.y) / 2);

	// Track
	int label_w = FromDIP(SLIDER_LABEL_WIDTH);
	int track_x = rect.GetLeft() + label_w;
	int track_w = rect.width - label_w - FromDIP(SLIDER_TEXT_MARGIN); // Room for value text
	int track_h = FromDIP(4);
	int track_y = rect.GetTop() + (rect.height - track_h) / 2;

	wxRect track_rect(track_x, track_y, track_w, track_h);

	dc.SetPen(*wxTRANSPARENT_PEN);
	dc.SetBrush(wxBrush(Theme::Get(Theme::Role::Border)));
	dc.DrawRectangle(track_rect);

	// Fill
	if (value > min && max > min) {
		float pct = std::clamp(static_cast<float>(value - min) / static_cast<float>(max - min), 0.0f, 1.0f);
		int fill_w = static_cast<int>(track_w * pct);
		wxRect fill_rect(track_x, track_y, fill_w, track_h);
		dc.SetBrush(wxBrush(Theme::Get(Theme::Role::Accent)));
		dc.DrawRectangle(fill_rect);

		// Thumb
		dc.SetBrush(wxBrush(Theme::Get(Theme::Role::Text)));
		dc.DrawCircle(track_x + fill_w, track_y + track_h / 2, FromDIP(SLIDER_THUMB_RADIUS));
	}

	// Value Text
	wxString val_str = std::format("{}", value);
	dc.DrawText(val_str, track_x + track_w + FromDIP(SLIDER_VALUE_MARGIN), rect.GetTop() + (rect.height - extent.y) / 2);
}

void ToolOptionsSurface::DrawCheckbox(wxDC& dc, const wxRect& rect, const wxString& label, bool value, bool hover) {
	// Box
	int box_sz = FromDIP(14);
	int box_y = rect.GetTop() + (rect.height - box_sz) / 2;
	wxRect box(rect.GetLeft(), box_y, box_sz, box_sz);

	if (hover) {
		dc.SetPen(wxPen(Theme::Get(Theme::Role::AccentHover)));
	} else {
		dc.SetPen(wxPen(Theme::Get(Theme::Role::Border)));
	}

	dc.SetBrush(value ? wxBrush(Theme::Get(Theme::Role::Accent)) : wxBrush(Theme::Get(Theme::Role::Background)));
	dc.DrawRectangle(box);

	// Checkmark (simple)
	if (value) {
		dc.SetPen(*wxWHITE_PEN);
		dc.DrawLine(box.GetLeft() + 3, box.GetTop() + 7, box.GetLeft() + 6, box.GetTop() + 10);
		dc.DrawLine(box.GetLeft() + 6, box.GetTop() + 10, box.GetRight() - 3, box.GetTop() + 4);
	}

	// Label
	dc.SetTextForeground(Theme::Get(Theme::Role::Text));
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

	bool hand_cursor = false;

	// Tools
	for (const auto& tr : tool_rects) {
		if (tr.rect.Contains(m_hoverPos)) {
			hover_brush = tr.brush;
			if (prev_hover != hover_brush) {
				SetToolTip(tr.tooltip);
			}
			hand_cursor = true;
			break;
		}
	}
	if (!hover_brush && prev_hover) {
		UnsetToolTip();
	}

	// Sliders Interaction
	if (evt.LeftDown()) {
		if (interactables.size_slider_rect.Contains(m_hoverPos)) {
			interactables.dragging_size = true;
			CaptureMouse();
			int val = CalculateSliderValue(interactables.size_slider_rect, MIN_BRUSH_SIZE, MAX_BRUSH_SIZE);
			if (val != current_size) {
				current_size = val;
				g_brush_manager.SetBrushSize(current_size);
				Refresh();
			}
			g_gui.SetStatusText(std::format("Brush Size: {}", current_size));
		} else if (interactables.thickness_slider_rect.Contains(m_hoverPos)) {
			interactables.dragging_thickness = true;
			CaptureMouse();
			int val = CalculateSliderValue(interactables.thickness_slider_rect, MIN_BRUSH_THICKNESS, MAX_BRUSH_THICKNESS);
			if (val != current_thickness) {
				current_thickness = val;
				g_brush_manager.SetBrushThickness(true, current_thickness, 100);
				Refresh();
			}
			g_gui.SetStatusText(std::format("Thickness: {}", current_thickness));
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
			g_gui.SetStatusText(std::format("Selected Tool: {}", hover_brush->getName()));
		}
	} else if (evt.LeftDClick()) {
		if (interactables.size_slider_rect.Contains(m_hoverPos)) {
			current_size = MIN_BRUSH_SIZE;
			g_brush_manager.SetBrushSize(current_size);
			Refresh();
			g_gui.SetStatusText(std::format("Brush Size: {}", current_size));
		} else if (interactables.thickness_slider_rect.Contains(m_hoverPos)) {
			current_thickness = MAX_BRUSH_THICKNESS;
			g_brush_manager.SetBrushThickness(true, current_thickness, 100);
			Refresh();
			g_gui.SetStatusText(std::format("Thickness: {}", current_thickness));
		}
	}

	if (evt.Dragging() && evt.LeftIsDown()) {
		if (interactables.dragging_size) {
			int val = CalculateSliderValue(interactables.size_slider_rect, MIN_BRUSH_SIZE, MAX_BRUSH_SIZE);
			if (val != current_size) {
				current_size = val;
				g_brush_manager.SetBrushSize(current_size); // Assuming square for now
				Refresh();
			}
			g_gui.SetStatusText(std::format("Brush Size: {}", current_size));
		}
		if (interactables.dragging_thickness) {
			int val = CalculateSliderValue(interactables.thickness_slider_rect, MIN_BRUSH_THICKNESS, MAX_BRUSH_THICKNESS);
			if (val != current_thickness) {
				current_thickness = val;
				g_brush_manager.SetBrushThickness(true, current_thickness, 100);
				Refresh();
			}
			g_gui.SetStatusText(std::format("Thickness: {}", current_thickness));
		}
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
		hand_cursor = true;
	}
	if (interactables.lock_check_rect.Contains(m_hoverPos)) {
		interactables.hover_lock = true;
		hand_cursor = true;
	}

	if (interactables.size_slider_rect.Contains(m_hoverPos) || interactables.thickness_slider_rect.Contains(m_hoverPos)) {
		hand_cursor = true;
	}

	SetCursor(hand_cursor ? wxCursor(wxCURSOR_HAND) : wxCursor(wxCURSOR_ARROW));

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

int ToolOptionsSurface::CalculateSliderValue(const wxRect& sliderRect, int min, int max) const {
	int label_w = FromDIP(SLIDER_LABEL_WIDTH);
	int track_x = sliderRect.GetLeft() + label_w;
	int track_w = sliderRect.width - label_w - FromDIP(SLIDER_TEXT_MARGIN);

	int rel_x = m_hoverPos.x - track_x;
	float pct = std::clamp(static_cast<float>(rel_x) / static_cast<float>(track_w), 0.0f, 1.0f);

	return min + static_cast<int>(pct * (max - min));
}
