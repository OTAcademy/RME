#include "app/main.h"
#include "palette/controls/virtual_brush_grid.h"
#include "ui/gui.h"
#include "rendering/core/graphics.h"

#include <glad/glad.h>

#include <nanovg.h>
#include <nanovg_gl.h>

#include <spdlog/spdlog.h>

VirtualBrushGrid::VirtualBrushGrid(wxWindow* parent, const TilesetCategory* _tileset, RenderSize rsz) :
	NanoVGCanvas(parent, wxID_ANY, wxVSCROLL | wxWANTS_CHARS),
	BrushBoxInterface(_tileset),
	icon_size(rsz),
	selected_index(-1),
	hover_index(-1),
	columns(1),
	item_size(0),
	padding(4),
	m_animTimer(new wxTimer(this)) {

	if (icon_size == RENDER_SIZE_16x16) {
		item_size = 18;
	} else {
		item_size = 34; // 32 + border
	}

	Bind(wxEVT_LEFT_DOWN, &VirtualBrushGrid::OnMouseDown, this);
	Bind(wxEVT_MOTION, &VirtualBrushGrid::OnMotion, this);
	Bind(wxEVT_TIMER, &VirtualBrushGrid::OnTimer, this);

	UpdateLayout();
}

VirtualBrushGrid::~VirtualBrushGrid() {
	m_animTimer->Stop();
	delete m_animTimer;
}

void VirtualBrushGrid::SetDisplayMode(DisplayMode mode) {
	if (display_mode != mode) {
		display_mode = mode;
		UpdateLayout();
		Refresh();
	}
}

void VirtualBrushGrid::UpdateLayout() {
	int width = GetClientSize().x;
	if (width <= 0) {
		width = 200; // Default
	}

	if (display_mode == DisplayMode::List) {
		columns = 1;
		int rowHeight = 36; // 32 icon + padding
		int rows = static_cast<int>(tileset->size());
		int contentHeight = rows * rowHeight + padding;
		UpdateScrollbar(contentHeight);
	} else {
		columns = std::max(1, (width - padding) / (item_size + padding));
		int rows = (static_cast<int>(tileset->size()) + columns - 1) / columns;
		int contentHeight = rows * (item_size + padding) + padding;
		UpdateScrollbar(contentHeight);
	}
}

wxSize VirtualBrushGrid::DoGetBestClientSize() const {
	return FromDIP(wxSize(200, 300));
}

void VirtualBrushGrid::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	// Update layout if needed
	if (display_mode == DisplayMode::List) {
		columns = 1;
		// Check if content height matches, logic simplified for now
	} else {
		int newCols = std::max(1, (width - padding) / (item_size + padding));
		if (newCols != columns) {
			columns = newCols;
			int rows = (static_cast<int>(tileset->size()) + columns - 1) / columns;
			int contentHeight = rows * (item_size + padding) + padding;
			UpdateScrollbar(contentHeight);
		}
	}

	// Calculate visible range
	int scrollPos = GetScrollPosition();
	int rowHeight = (display_mode == DisplayMode::List) ? 36 : (item_size + padding);
	int startRow = scrollPos / rowHeight;
	int endRow = (scrollPos + height + rowHeight - 1) / rowHeight + 1;

	int startIdx = startRow * columns;
	int endIdx = std::min(static_cast<int>(tileset->size()), endRow * columns);

	// Draw visible items
	for (int i = startIdx; i < endIdx; ++i) {
		DrawBrushItem(vg, i, GetItemRect(i));
	}
}

void VirtualBrushGrid::DrawBrushItem(NVGcontext* vg, int i, const wxRect& rect) {
	float x = static_cast<float>(rect.x);
	float y = static_cast<float>(rect.y);
	float w = static_cast<float>(rect.width);
	float h = static_cast<float>(rect.height);

	// Shadow / Glow
	if (i == selected_index) {
		// Glow for selected
		NVGpaint shadowPaint = nvgBoxGradient(vg, x, y, w, h, 4.0f, 10.0f, nvgRGBA(100, 150, 255, 128), nvgRGBA(0, 0, 0, 0));
		nvgBeginPath(vg);
		nvgRect(vg, x - 10, y - 10, w + 20, h + 20);
		nvgRoundedRect(vg, x, y, w, h, 4.0f);
		nvgPathWinding(vg, NVG_HOLE);
		nvgFillPaint(vg, shadowPaint);
		nvgFill(vg);
	} else if (i == hover_index) {
		// Subtle shadow/glow for hover
		NVGpaint shadowPaint = nvgBoxGradient(vg, x, y + 2, w, h, 4.0f, 6.0f, nvgRGBA(0, 0, 0, 64), nvgRGBA(0, 0, 0, 0));
		nvgBeginPath(vg);
		nvgRect(vg, x - 5, y - 5, w + 10, h + 10);
		nvgRoundedRect(vg, x, y, w, h, 4.0f);
		nvgPathWinding(vg, NVG_HOLE);
		nvgFillPaint(vg, shadowPaint);
		nvgFill(vg);
	}

	// Card background
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x, y, w, h, 4.0f);

	if (i == selected_index) {
		nvgFillColor(vg, nvgRGBA(80, 100, 120, 255));
	} else if (i == hover_index) {
		nvgFillColor(vg, nvgRGBA(70, 70, 75, 255));
	} else {
		// Normal - dark card with subtle gradient
		NVGpaint bgPaint = nvgLinearGradient(vg, x, y, x, y + h, nvgRGBA(60, 60, 65, 255), nvgRGBA(50, 50, 55, 255));
		nvgFillPaint(vg, bgPaint);
	}
	nvgFill(vg);

	// Selection border
	if (i == selected_index) {
		nvgBeginPath(vg);
		nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1.0f, h - 1.0f, 4.0f);
		nvgStrokeColor(vg, nvgRGBA(100, 180, 255, 255));
		nvgStrokeWidth(vg, 2.0f);
		nvgStroke(vg);
	}

	// Draw brush sprite
	Brush* brush = (i < static_cast<int>(tileset->size())) ? tileset->brushlist[i] : nullptr;
	if (brush) {
		Sprite* spr = brush->getSprite();
		if (!spr) {
			spr = g_gui.gfx.getSprite(brush->getLookID());
		}

		int tex = GetOrCreateSpriteTexture(vg, spr);
		if (tex > 0) {
			int iconSize = (display_mode == DisplayMode::List) ? 32 : (item_size - 4);
			int iconX = rect.x + 2;
			int iconY = (display_mode == DisplayMode::List) ? (rect.y + 2) : (rect.y + 2);

			NVGpaint imgPaint = nvgImagePattern(vg, static_cast<float>(iconX), static_cast<float>(iconY), static_cast<float>(iconSize), static_cast<float>(iconSize), 0.0f, tex, 1.0f);

			nvgBeginPath(vg);
			nvgRoundedRect(vg, static_cast<float>(iconX), static_cast<float>(iconY), static_cast<float>(iconSize), static_cast<float>(iconSize), 3.0f);
			nvgFillPaint(vg, imgPaint);
			nvgFill(vg);
		}

		if (display_mode == DisplayMode::List) {
			nvgFontSize(vg, 14.0f);
			nvgFontFace(vg, "sans");
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
			nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));

			wxString name = wxstr(brush->getName());
			nvgText(vg, rect.x + 40, rect.y + rect.height / 2.0f, name.ToUTF8().data(), nullptr);
		}
	}
}

wxRect VirtualBrushGrid::GetItemRect(int index) const {
	if (display_mode == DisplayMode::List) {
		int width = GetClientSize().x - 2 * padding;
		int rowHeight = 36;
		return wxRect(padding, padding + index * rowHeight, width, 36);
	} else {
		int row = index / columns;
		int col = index % columns;

		return wxRect(
			padding + col * (item_size + padding),
			padding + row * (item_size + padding),
			item_size,
			item_size
		);
	}
}

int VirtualBrushGrid::HitTest(int x, int y) const {
	int scrollPos = GetScrollPosition();
	int realY = y + scrollPos;
	int realX = x;

	if (display_mode == DisplayMode::List) {
		int rowHeight = 36;
		int row = (realY - padding) / rowHeight;

		if (row < 0 || row >= static_cast<int>(tileset->size())) {
			return -1;
		}

		int index = row;
		// Check horizontal bounds properly
		int width = GetClientSize().x; // roughly
		if (realX >= padding && realX <= width - padding) {
			return index;
		}
		return -1;
	} else {
		int col = (realX - padding) / (item_size + padding);
		int row = (realY - padding) / (item_size + padding);

		if (col < 0 || col >= columns || row < 0) {
			return -1;
		}

		int index = row * columns + col;
		if (index >= 0 && index < static_cast<int>(tileset->size())) {
			wxRect rect = GetItemRect(index);
			// Adjust rect to scroll position for contains check
			rect.y -= scrollPos;
			if (rect.Contains(x, y)) {
				return index;
			}
		}
		return -1;
	}
}

void VirtualBrushGrid::OnMouseDown(wxMouseEvent& event) {
	int index = HitTest(event.GetX(), event.GetY());
	if (index != -1 && index != selected_index) {
		selected_index = index;

		// Notify GUI - find PaletteWindow parent
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

void VirtualBrushGrid::OnMotion(wxMouseEvent& event) {
	int index = HitTest(event.GetX(), event.GetY());

	if (index != hover_index) {
		hover_index = index;
		Refresh();
	}

	// Tooltip
	if (index != -1) {
		Brush* brush = tileset->brushlist[index];
		if (brush) {
			wxString tip = wxstr(brush->getName());
			if (GetToolTipText() != tip) {
				SetToolTip(tip);
			}
		}
	} else {
		UnsetToolTip();
	}

	event.Skip();
}

void VirtualBrushGrid::OnTimer(wxTimerEvent& event) {
	// Animation tick for hover effects (optional - can be enhanced later)
	Refresh();
}

void VirtualBrushGrid::SelectFirstBrush() {
	if (tileset->size() > 0) {
		selected_index = 0;
		Refresh();
	}
}

Brush* VirtualBrushGrid::GetSelectedBrush() const {
	if (selected_index >= 0 && selected_index < static_cast<int>(tileset->size())) {
		return tileset->brushlist[selected_index];
	}
	return nullptr;
}

bool VirtualBrushGrid::SelectBrush(const Brush* brush) {
	for (size_t i = 0; i < tileset->size(); ++i) {
		if (tileset->brushlist[i] == brush) {
			selected_index = static_cast<int>(i);

			// Ensure visible
			wxRect rect = GetItemRect(selected_index);
			int scrollPos = GetScrollPosition();
			int clientHeight = GetClientSize().y;

			if (rect.y < scrollPos) {
				SetScrollPosition(rect.y - padding);
			} else if (rect.y + rect.height > scrollPos + clientHeight) {
				SetScrollPosition(rect.y + rect.height - clientHeight + padding);
			}

			Refresh();
			return true;
		}
	}
	selected_index = -1;
	Refresh();
	return false;
}
