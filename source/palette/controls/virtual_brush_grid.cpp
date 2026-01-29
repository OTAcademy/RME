#include "palette/controls/virtual_brush_grid.h"
#include "ui/gui.h"
#include <wx/dcbuffer.h>
#include <spdlog/spdlog.h>

BEGIN_EVENT_TABLE(VirtualBrushGrid, wxScrolledWindow)
EVT_PAINT(VirtualBrushGrid::OnPaint)
EVT_SIZE(VirtualBrushGrid::OnSize)
EVT_LEFT_DOWN(VirtualBrushGrid::OnMouse)
EVT_ERASE_BACKGROUND(VirtualBrushGrid::OnEraseBackground)
END_EVENT_TABLE()

VirtualBrushGrid::VirtualBrushGrid(wxWindow* parent, const TilesetCategory* _tileset, RenderSize rsz) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxWANTS_CHARS),
	BrushBoxInterface(_tileset),
	icon_size(rsz),
	selected_index(-1),
	columns(1),
	padding(4),
	is_dragging(false) {
	if (icon_size == RENDER_SIZE_16x16) {
		item_size = 18;
	} else {
		item_size = 34; // 32 + border
	}

	SetBackgroundStyle(wxBG_STYLE_PAINT);
	UpdateVirtualSize();
}

VirtualBrushGrid::~VirtualBrushGrid() {
}

void VirtualBrushGrid::UpdateVirtualSize() {
	int width, height;
	GetClientSize(&width, &height);

	columns = std::max(1, (width - padding) / (item_size + padding));
	int rows = (tileset->size() + columns - 1) / columns;

	SetScrollbars(0, item_size + padding, 0, rows, 0, 0);
	Refresh();
}

void VirtualBrushGrid::OnSize(wxSizeEvent& event) {
	UpdateVirtualSize();
	event.Skip();
}

void VirtualBrushGrid::OnEraseBackground(wxEraseEvent& event) {
	// Do nothing to prevent flicker
}

void VirtualBrushGrid::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC(dc);

	// Clear background
	dc.SetBackground(wxBrush(GetBackgroundColour()));
	dc.Clear();

	int unit_x, unit_y;
	GetViewStart(&unit_x, &unit_y);
	int view_y = unit_y * (item_size + padding);

	int cw, ch;
	GetClientSize(&cw, &ch);

	int start_row = unit_y;
	int end_row = (view_y + ch + item_size + padding - 1) / (item_size + padding);

	int start_idx = start_row * columns;
	int end_idx = std::min((int)tileset->size(), (end_row + 1) * columns);

	SpriteSize spr_sz = (icon_size == RENDER_SIZE_16x16) ? SPRITE_SIZE_16x16 : SPRITE_SIZE_32x32;
	for (int i = start_idx; i < end_idx; ++i) {
		wxRect rect = GetItemRect(i);

		// Highlight selection
		if (i == selected_index) {
			dc.SetPen(*wxWHITE_PEN);
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
			dc.DrawRectangle(rect);
		}

		// Draw sprite
		Brush* brush = tileset->brushlist[i];
		if (brush) {
			Sprite* spr = g_gui.gfx.getSprite(brush->getLookID());
			if (spr) {
				spr->DrawTo(&dc, spr_sz, rect.x + 1, rect.y + 1, item_size - 2, item_size - 2);
			}
		}
	}
}

wxRect VirtualBrushGrid::GetItemRect(int index) const {
	int row = index / columns;
	int col = index % columns;

	return wxRect(
		padding + col * (item_size + padding),
		padding + row * (item_size + padding),
		item_size,
		item_size
	);
}

int VirtualBrushGrid::HitTest(int x, int y) const {
	int unit_x, unit_y;
	GetViewStart(&unit_x, &unit_y);

	int real_y = y + (unit_y * (item_size + padding));
	int real_x = x; // Horizontal scroll is 0

	int col = (real_x - padding) / (item_size + padding);
	int row = (real_y - padding) / (item_size + padding);

	if (col < 0 || col >= columns || row < 0) {
		return -1;
	}

	int index = row * columns + col;
	if (index >= 0 && index < (int)tileset->size()) {
		// Verify if it's actually within the square, not in padding
		wxRect rect = GetItemRect(index);
		if (rect.Contains(real_x, real_y)) {
			return index;
		}
	}

	return -1;
}

void VirtualBrushGrid::OnMouse(wxMouseEvent& event) {
	int index = HitTest(event.GetX(), event.GetY());
	if (index != -1 && index != selected_index) {
		selected_index = index;

		// Notify GUI
		// We need to find the PaletteWindow parent
		wxWindow* w = GetParent();
		while (w) {
			PaletteWindow* pw = dynamic_cast<PaletteWindow*>(w);
			if (pw) {
				g_gui.ActivatePalette(pw);
				break;
			}
			w = w->GetParent();
		}

		g_gui.SelectBrush(tileset->brushlist[selected_index], tileset->getType());
		Refresh();
	}
}

void VirtualBrushGrid::SelectFirstBrush() {
	if (tileset->size() > 0) {
		selected_index = 0;
		Refresh();
	}
}

Brush* VirtualBrushGrid::GetSelectedBrush() const {
	if (selected_index >= 0 && selected_index < (int)tileset->size()) {
		return tileset->brushlist[selected_index];
	}
	return nullptr;
}

bool VirtualBrushGrid::SelectBrush(const Brush* brush) {
	for (size_t i = 0; i < tileset->size(); ++i) {
		if (tileset->brushlist[i] == brush) {
			selected_index = (int)i;

			// Ensure visible logic could be added here
			int view_x, view_y;
			GetViewStart(&view_x, &view_y);
			int cw, ch;
			GetClientSize(&cw, &ch);

			int view_y_px = view_y * (item_size + padding);

			wxRect rect = GetItemRect(selected_index);
			if (rect.y < view_y_px || rect.y + rect.height > view_y_px + ch) {
				Scroll(-1, (rect.y - padding) / (item_size + padding));
			}

			Refresh();
			return true;
		}
	}
	selected_index = -1;
	Refresh();
	return false;
}
