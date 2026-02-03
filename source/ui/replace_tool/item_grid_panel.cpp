#include "ui/replace_tool/item_grid_panel.h"
#include "ui/theme.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <algorithm>

ItemGridPanel::ItemGridPanel(wxWindow* parent, Listener* listener) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxWANTS_CHARS),
	listener(listener), m_animTimer(this) {

	SetBackgroundStyle(wxBG_STYLE_PAINT);
	// Match OutfitSelectionGrid background
	SetBackgroundColour(wxColour(45, 45, 45));

	Bind(wxEVT_PAINT, &ItemGridPanel::OnPaint, this);
	Bind(wxEVT_SIZE, &ItemGridPanel::OnSize, this);
	Bind(wxEVT_TIMER, &ItemGridPanel::OnTimer, this);
	Bind(wxEVT_LEFT_DOWN, &ItemGridPanel::OnMouse, this);
	Bind(wxEVT_MOTION, &ItemGridPanel::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &ItemGridPanel::OnMouse, this);

	Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) { /* No-op */ });

	// Set scroll rate to match the item height + padding (row-based scrolling)
	SetScrollRate(0, item_height + padding);
}

ItemGridPanel::~ItemGridPanel() { }

void ItemGridPanel::SetItems(const std::vector<uint16_t>& items) {
	allItems = items;
	m_nameOverrides.clear();
	SetFilter("");
}

void ItemGridPanel::SetOverrideNames(const std::map<uint16_t, wxString>& names) {
	m_nameOverrides = names;
	Refresh();
}

void ItemGridPanel::SetFilter(const wxString& filter) {
	filteredItems.clear();
	wxString lowerFilter = filter.Lower();

	for (uint16_t id : allItems) {
		if (id < 100) {
			continue;
		}

		if (lowerFilter.IsEmpty()) {
			filteredItems.push_back(id);
		} else {
			const ItemType& it = g_items.getItemType(id);
			wxString name = wxString(it.name);
			wxString sid = wxString::Format("%d", id);
			wxString cid = wxString::Format("%d", it.clientID);

			if (name.Lower().Contains(lowerFilter) || sid.Contains(lowerFilter) || cid.Contains(lowerFilter)) {
				filteredItems.push_back(id);
			}
		}
	}
	UpdateVirtualSize();
	Refresh();
}

void ItemGridPanel::UpdateVirtualSize() {
	int width, height;
	GetClientSize(&width, &height);

	m_cols = std::max(1, (width - padding) / (item_width + padding));
	int rows = (filteredItems.size() + m_cols - 1) / m_cols;

	// Set scrollbars: unitX, unitY, noUnitsX, noUnitsY
	SetScrollbars(0, item_height + padding, 0, rows, 0, 0);
}

wxSize ItemGridPanel::DoGetBestClientSize() const {
	return wxSize(FromDIP(240), FromDIP(300));
}

void ItemGridPanel::OnSize(wxSizeEvent& event) {
	UpdateVirtualSize();
	event.Skip();
}

void ItemGridPanel::SetShowDetails(bool show) {
	m_showDetails = show;
	Refresh();
}

void ItemGridPanel::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC(dc); // adjust DC origin for scrolling

	dc.SetBackground(wxBrush(GetBackgroundColour()));
	dc.Clear();

	int unit_x, unit_y;
	GetViewStart(&unit_x, &unit_y);
	// unit_y is the row index we are scrolled to

	int cw, ch;
	GetClientSize(&cw, &ch);

	// Calculate visible range
	// Since 1 scroll unit = 1 row (item_height + padding)
	int start_row = unit_y;
	// How many pixels visible?
	int view_h = ch;
	// How many rows fit in view_h?
	int visible_rows = (view_h + (item_height + padding) - 1) / (item_height + padding);
	int end_row = start_row + visible_rows + 1; // +1 buffer

	size_t start_idx = (size_t)start_row * m_cols;
	size_t end_idx = std::min(filteredItems.size(), (size_t)(end_row + 1) * m_cols);

	// Setup font
	wxFont font = GetFont();
	font.SetPointSize(8);
	dc.SetFont(font);

	for (size_t i = start_idx; i < end_idx; ++i) {
		uint16_t id = filteredItems[i];
		wxRect rect = GetItemRect(i);

		// "Card" styling matching OutfitSelectionGrid
		bool isSelected = (id == selectedId);

		if (isSelected) {
			dc.SetPen(wxPen(wxColour(200, 200, 200), 2));
			dc.SetBrush(wxBrush(wxColour(80, 80, 80)));
		} else {
			if (id == hoveredId) {
				dc.SetPen(wxPen(wxColour(100, 100, 100), 1));
				dc.SetBrush(wxBrush(wxColour(60, 60, 60)));
			} else {
				dc.SetPen(wxPen(wxColour(60, 60, 60), 1));
				dc.SetBrush(wxBrush(wxColour(50, 50, 50)));
			}
		}
		dc.DrawRectangle(rect);

		const ItemType& it = g_items.getItemType(id);

		// 1. Sprite -- Top centered (y: 6 to 38)
		Sprite* sprite = g_gui.gfx.getSprite(it.clientID);
		if (sprite && g_version.getLoadedVersion()) {
			int ix = rect.x + (rect.width - 32) / 2;
			int iy = rect.y + 6;
			sprite->DrawTo(&dc, SPRITE_SIZE_32x32, ix, iy);
		}

		// Draw Text
		int tw, th;

		// 2. Name (y: 45)
		wxFont nameFont = GetFont();
		nameFont.SetPointSize(8);
		dc.SetFont(nameFont);
		dc.SetTextForeground(*wxWHITE);

		wxString label;
		auto itName = m_nameOverrides.find(id);
		if (itName != m_nameOverrides.end()) {
			label = itName->second;
		} else {
			label = it.name;
		}

		if (label.Length() > 14) {
			label = label.Left(12) + "..";
		}
		dc.GetTextExtent(label, &tw, &th);
		dc.DrawText(label, rect.x + (rect.width - tw) / 2, rect.y + 45);

		// 3. Server ID & Client ID (Only if showDetails is true)
		if (m_showDetails) {
			wxFont detailFont = GetFont();
			detailFont.SetPointSize(7);
			dc.SetFont(detailFont);

			// SID (y: 65)
			wxString sidStr = wxString::Format("S: %d", id);
			dc.SetTextForeground(wxColour(180, 180, 180));
			dc.GetTextExtent(sidStr, &tw, &th);
			dc.DrawText(sidStr, rect.x + (rect.width - tw) / 2, rect.y + 65);

			// CID (y: 80)
			wxString cidStr = wxString::Format("C: %d", it.clientID);
			dc.SetTextForeground(wxColour(150, 150, 150));
			dc.GetTextExtent(cidStr, &tw, &th);
			dc.DrawText(cidStr, rect.x + (rect.width - tw) / 2, rect.y + 80);
		}
	}
}

wxRect ItemGridPanel::GetItemRect(int index) const {
	int row = index / m_cols;
	int col = index % m_cols;

	return wxRect(
		padding + col * (item_width + padding),
		padding + row * (item_height + padding),
		item_width,
		item_height
	);
}

int ItemGridPanel::HitTest(int x, int y) const {
	int unit_x, unit_y;
	GetViewStart(&unit_x, &unit_y);

	// Convert logical y to device y relative to start?
	// Wait, Mouse Event gives device coordinates.
	// But GetItemRect returns logical coordinates (if we consider 0,0 top of buffer).
	// wxScrolledWindow with DoPrepareDC automatically translates DC coords.
	// But specific HitTest usually needs manual adjustment if using simple rects.

	// Better approach for HitTest with scrolling:
	// "Unscroll" the mouse position
	int logical_y = y + (unit_y * (item_height + padding));
	int logical_x = x; // Horizontal scrolling disabled

	int col = (logical_x - padding) / (item_width + padding);
	int row = (logical_y - padding) / (item_height + padding);

	if (col < 0 || col >= m_cols || row < 0) {
		return -1;
	}

	int index = row * m_cols + col;
	if (index >= 0 && index < (int)filteredItems.size()) {
		// Verify exact containment within the card rect (ignoring padding gap)
		wxRect r = GetItemRect(index);
		// GetItemRect returns logical coords.
		if (r.Contains(logical_x, logical_y)) {
			return index;
		}
	}
	return -1;
}

void ItemGridPanel::OnMotion(wxMouseEvent& event) {
	// Update cursor to hand if over an item
	int idx = HitTest(event.GetX(), event.GetY());
	if (idx != -1) {
		SetCursor(wxCursor(wxCURSOR_HAND));
		uint16_t id = filteredItems[idx];
		if (hoveredId != id) {
			hoveredId = id;
			Refresh();
		}
	} else {
		SetCursor(wxNullCursor);
		if (hoveredId != 0) {
			hoveredId = 0;
			Refresh();
		}
	}

	// Handle Dragging
	if (event.Dragging() && m_draggable && selectedId != 0) {
		wxString payload = wxString::Format("RME_ITEM:%d", selectedId);
		wxTextDataObject data(payload);
		wxDropSource dragSource(this);
		dragSource.SetData(data);
		dragSource.DoDragDrop(wxDrag_AllowMove);
	}

	event.Skip();
}

void ItemGridPanel::OnMouse(wxMouseEvent& event) {
	if (event.LeftDown()) {
		int idx = HitTest(event.GetX(), event.GetY());
		if (idx != -1) {
			selectedId = filteredItems[idx];
			if (listener) {
				listener->OnItemSelected(this, selectedId);
			}
			Refresh();
		}
	}
	// Note: Drag start is handled in OnMotion to avoid conflict with click
	// But standard wx DnD often starts in Motion or purely LeftDown check.
	// We'll keep the motion check for dragging as it allows "click to select" vs "click and move to drag".
}

void ItemGridPanel::OnTimer(wxTimerEvent& event) {
	// Not strictly needed with new style, but keeping if we want to add fade later
	m_animTimer.Stop();
}

void ItemGridPanel::SetDraggable(bool check) {
	m_draggable = check;
}
