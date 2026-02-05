#include "util/virtual_item_grid.h"
#include "util/nvg_utils.h"
#include "ui/theme.h"
#include <algorithm>
#include <format>
#include <cmath>

VirtualItemGrid::VirtualItemGrid(wxWindow* parent, wxWindowID id) :
	NanoVGCanvas(parent, id, wxVSCROLL | wxWANTS_CHARS),
	m_itemSize(80), // Larger for text area
	m_padding(6),
	m_columns(1),
	m_selectedIndex(-1),
	m_hoverIndex(-1),
	m_animTimer(this) {
	Bind(wxEVT_SIZE, &VirtualItemGrid::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &VirtualItemGrid::OnMouseDown, this);
	Bind(wxEVT_MOTION, &VirtualItemGrid::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &VirtualItemGrid::OnLeave, this);

	// Optional timer for smooth effects if needed
	// Bind(wxEVT_TIMER, &VirtualItemGrid::OnTimer, this);
}

VirtualItemGrid::~VirtualItemGrid() {
}

void VirtualItemGrid::RefreshGrid() {
	UpdateLayout();
	Refresh();
}

void VirtualItemGrid::UpdateLayout() {
	int width = GetClientSize().x;
	if (width <= 0) {
		width = 200;
	}

	m_columns = std::max(1, (width - m_padding) / (m_itemSize + m_padding));
	size_t count = GetItemCount();
	int rows = (count + m_columns - 1) / m_columns;
	int contentHeight = rows * (m_itemSize + m_padding) + m_padding;

	UpdateScrollbar(contentHeight);
}

wxSize VirtualItemGrid::DoGetBestClientSize() const {
	return FromDIP(wxSize(250, 400));
}

int VirtualItemGrid::HitTest(int x, int y) const {
	int scrollPos = GetScrollPosition();
	int realY = y + scrollPos;
	int col = (x - m_padding) / (m_itemSize + m_padding);
	int row = (realY - m_padding) / (m_itemSize + m_padding);

	if (col < 0 || col >= m_columns || row < 0) {
		return -1;
	}

	size_t index = row * m_columns + col;
	if (index < GetItemCount()) {
		wxRect r = GetItemRect((int)index);
		r.y -= scrollPos; // Convert back to local for test
		if (r.Contains(x, y)) {
			return (int)index;
		}
	}
	return -1;
}

wxRect VirtualItemGrid::GetItemRect(int index) const {
	int row = index / m_columns;
	int col = index % m_columns;
	return wxRect(
		m_padding + col * (m_itemSize + m_padding),
		m_padding + row * (m_itemSize + m_padding),
		m_itemSize,
		m_itemSize
	);
}

void VirtualItemGrid::SetSelection(int index) {
	if (index != m_selectedIndex) {
		m_selectedIndex = index;
		OnItemSelected(index);
		EnsureVisible(index);
		Refresh();
	}
}

uint16_t VirtualItemGrid::GetSelectedItemId() const {
	if (m_selectedIndex >= 0 && m_selectedIndex < (int)GetItemCount()) {
		return GetItem(m_selectedIndex);
	}
	return 0;
}

void VirtualItemGrid::EnsureVisible(int index) {
	if (index < 0 || index >= (int)GetItemCount()) {
		return;
	}

	wxRect rect = GetItemRect(index);
	int scrollPos = GetScrollPosition();
	int clientH = GetClientSize().y;

	if (rect.y < scrollPos) {
		SetScrollPosition(rect.y - m_padding);
	} else if (rect.y + rect.height > scrollPos + clientH) {
		SetScrollPosition(rect.y + rect.height - clientH + m_padding);
	}
}

void VirtualItemGrid::OnSize(wxSizeEvent& event) {
	UpdateLayout();
	Refresh();
	event.Skip();
}

void VirtualItemGrid::OnMouseDown(wxMouseEvent& event) {
	int index = HitTest(event.GetX(), event.GetY());
	if (index != -1) {
		SetSelection(index);
	}
	event.Skip();
}

void VirtualItemGrid::OnMotion(wxMouseEvent& event) {
	int index = HitTest(event.GetX(), event.GetY());
	if (index != m_hoverIndex) {
		m_hoverIndex = index;

		if (m_hoverIndex != -1) {
			SetCursor(wxCursor(wxCURSOR_HAND));
			SetToolTip(GetItemName(m_hoverIndex));
		} else {
			SetCursor(wxNullCursor);
			UnsetToolTip();
		}
		Refresh();
	}
	event.Skip();
}

void VirtualItemGrid::OnLeave(wxMouseEvent& event) {
	if (m_hoverIndex != -1) {
		m_hoverIndex = -1;
		Refresh();
	}
	event.Skip();
}

void VirtualItemGrid::OnTimer(wxTimerEvent&) {
}

int VirtualItemGrid::GetCachedTexture(uint16_t id) {
	for (const auto& entry : m_textureCache) {
		if (entry.id == id) {
			return entry.tex;
		}
	}
	return 0;
}

void VirtualItemGrid::AddCachedTexture(uint16_t id, int tex) {
	m_textureCache.push_back({ id, tex });
}

void VirtualItemGrid::ClearCache() {
	m_textureCache.clear();
}

int VirtualItemGrid::GetOrCreateItemTexture(NVGcontext* vg, uint16_t id) {
	int tex = GetCachedTexture(id);
	if (tex > 0) {
		return tex;
	}

	tex = NvgUtils::CreateItemTexture(vg, id);
	if (tex > 0) {
		AddCachedTexture(id, tex);
	}
	return tex;
}

wxString VirtualItemGrid::GetItemName(size_t index) const {
	if (index < GetItemCount()) {
		uint16_t id = GetItem(index);
		return g_items.getItemType(id).name;
	}
	return "";
}

void VirtualItemGrid::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	int rowHeight = m_itemSize + m_padding;
	int scrollPos = GetScrollPosition();

	size_t count = GetItemCount();
	if (count == 0) {
		return;
	}

	// Calculate visible range
	int startRow = scrollPos / rowHeight;
	int endRow = (scrollPos + height + rowHeight - 1) / rowHeight;

	int startIdx = startRow * m_columns;
	int endIdx = std::min((int)count, (endRow + 1) * m_columns);

	for (int i = startIdx; i < endIdx; ++i) {
		if (i < 0) {
			continue;
		}

		wxRect r = GetItemRect(i);
		float x = (float)r.x;
		float y = (float)r.y;
		float w = (float)r.width;
		float h = (float)r.height;

		bool isSelected = (i == m_selectedIndex);
		bool isHovered = (i == m_hoverIndex);

		// Shadow / Glow
		if (isSelected) {
			NVGpaint shadowPaint = nvgBoxGradient(vg, x, y, w, h, 4.0f, 10.0f, nvgRGBA(100, 150, 255, 128), nvgRGBA(0, 0, 0, 0));
			nvgBeginPath(vg);
			nvgRect(vg, x - 10, y - 10, w + 20, h + 20);
			nvgRoundedRect(vg, x, y, w, h, 4.0f);
			nvgPathWinding(vg, NVG_HOLE);
			nvgFillPaint(vg, shadowPaint);
			nvgFill(vg);
		} else if (isHovered) {
			NVGpaint shadowPaint = nvgBoxGradient(vg, x, y + 2, w, h, 4.0f, 6.0f, nvgRGBA(0, 0, 0, 64), nvgRGBA(0, 0, 0, 0));
			nvgBeginPath(vg);
			nvgRect(vg, x - 5, y - 5, w + 10, h + 10);
			nvgRoundedRect(vg, x, y, w, h, 4.0f);
			nvgPathWinding(vg, NVG_HOLE);
			nvgFillPaint(vg, shadowPaint);
			nvgFill(vg);
		}

		// Card Background
		nvgBeginPath(vg);
		nvgRoundedRect(vg, x, y, w, h, 4.0f);
		if (isSelected) {
			nvgFillColor(vg, nvgRGBA(80, 100, 120, 255));
		} else if (isHovered) {
			nvgFillColor(vg, nvgRGBA(70, 70, 75, 255));
		} else {
			NVGpaint bgPaint = nvgLinearGradient(vg, x, y, x, y + h, nvgRGBA(60, 60, 65, 255), nvgRGBA(50, 50, 55, 255));
			nvgFillPaint(vg, bgPaint);
		}
		nvgFill(vg);

		// Selection Border
		if (isSelected) {
			nvgBeginPath(vg);
			nvgRoundedRect(vg, x + 0.5f, y + 0.5f, w - 1, h - 1, 4.0f);
			nvgStrokeColor(vg, nvgRGBA(100, 180, 255, 255));
			nvgStrokeWidth(vg, 2.0f);
			nvgStroke(vg);
		}

		// Draw Item Icon (Fixed 32x32 size for consistency)
		uint16_t id = GetItem(i);
		int tex = GetOrCreateItemTexture(vg, id);
		if (tex > 0) {
			// Fixed 32x32 rendering area
			float iconSize = 32.0f;
			int tw, th;
			nvgImageSize(vg, tex, &tw, &th);

			float scale = iconSize / std::max(tw, th);
			if (scale > 1.0f && std::max(tw, th) >= 32) {
				scale = 1.0f;
			}

			float dw = tw * scale;
			float dh = th * scale;
			float dx = x + (w - dw) / 2.0f;
			float dy = y + 8.0f; // Top padding

			NVGpaint imgPaint = nvgImagePattern(vg, dx, dy, dw, dh, 0.0f, tex, 1.0f);
			nvgBeginPath(vg);
			nvgRect(vg, dx, dy, dw, dh);
			nvgFillPaint(vg, imgPaint);
			nvgFill(vg);
		}

		// Draw Text (2 Lines)
		wxString name = GetItemName(i);
		if (!name.IsEmpty()) {
			nvgFontSize(vg, 12.0f); // Larger font
			nvgFontFace(vg, "sans");
			nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
			if (isSelected) {
				nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
			} else {
				nvgFillColor(vg, nvgRGBA(200, 200, 200, 255));
			}

			float textY = y + 44.0f; // Below the 32px icon area + padding
			float maxWidth = w - 4.0f;

			// NvgTextBox approach for wrapping
			nvgTextBox(vg, x + 2, textY, maxWidth, name.mb_str(), nullptr);
		}
	}
}
