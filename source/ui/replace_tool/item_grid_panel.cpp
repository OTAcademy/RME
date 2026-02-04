#include "ui/replace_tool/item_grid_panel.h"
#include "ui/theme.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include <wx/dcclient.h>
#include <wx/graphics.h>
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <algorithm>
#include <iostream>
#include <format>
#include "rendering/core/text_renderer.h"
#include "util/nvg_utils.h"

ItemGridPanel::ItemGridPanel(wxWindow* parent, Listener* listener) :
	NanoVGCanvas(parent, wxID_ANY),
	listener(listener), m_animTimer(this) {

	Bind(wxEVT_SIZE, &ItemGridPanel::OnSize, this);
	Bind(wxEVT_TIMER, &ItemGridPanel::OnTimer, this);
	Bind(wxEVT_LEFT_DOWN, &ItemGridPanel::OnMouse, this);
	Bind(wxEVT_MOTION, &ItemGridPanel::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &ItemGridPanel::OnMouse, this);
}

ItemGridPanel::~ItemGridPanel() {
}

int ItemGridPanel::GetTextureForId(uint16_t id) {
	int tex = GetCachedImage(id);
	if (tex > 0) {
		return tex;
	}

	tex = NvgUtils::CreateItemTexture(GetNVGContext(), id);
	if (tex > 0) {
		AddCachedImage(id, tex);
	}
	return tex;
}

void ItemGridPanel::SetItems(const std::vector<uint16_t>& items) {
	allItems = items;
	m_nameOverrides.clear();
	ClearImageCache();
	SetFilter("");
}

void ItemGridPanel::SetFilter(const wxString& filter) {
	filteredItems.clear();
	wxString lowerFilter = filter.Lower();
	for (uint16_t id : allItems) {
		if (id < 100) {
			continue;
		}
		const ItemType& it = g_items.getItemType(id);
		if (lowerFilter.IsEmpty()) {
			filteredItems.push_back(id);
		} else {
			wxString name = wxString(it.name);
			if (name.Lower().Contains(lowerFilter) || wxString(std::format("{}", id)).Contains(lowerFilter) || wxString(std::format("{}", it.clientID)).Contains(lowerFilter)) {
				filteredItems.push_back(id);
			}
		}
	}
	SetScrollPosition(0);
	UpdateVirtualSize();
	Refresh();
}

void ItemGridPanel::UpdateVirtualSize() {
	int width, height;
	GetClientSize(&width, &height);
	m_cols = std::max(1, (width - padding) / (item_width + padding));
	m_maxRows = (int)((filteredItems.size() + m_cols - 1) / m_cols);

	int totalHeight = m_maxRows * (item_height + padding);
	UpdateScrollbar(totalHeight);
}

void ItemGridPanel::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	int rowH = item_height + padding;
	int scrollPos = GetScrollPosition();

	int startRow = scrollPos / rowH;
	int endRow = startRow + (height / rowH) + 2;

	size_t startIdx = (size_t)startRow * m_cols;
	size_t endIdx = std::min(filteredItems.size(), (size_t)endRow * m_cols);

	for (size_t i = startIdx; i < endIdx; ++i) {
		uint16_t id = filteredItems[i];
		wxRect r = GetItemRect(i);
		// Note: GetItemRect returns coordinates relative to top (as if scrollPos=0)
		// NanoVGCanvas translates by -scrollPos, so we draw at r.x, r.y directly.

		// Card
		nvgBeginPath(vg);
		nvgRoundedRect(vg, r.x, r.y, r.width, r.height, 4.0f);
		if (id == selectedId) {
			nvgFillColor(vg, nvgRGBA(80, 80, 80, 255));
			nvgFill(vg);
			nvgStrokeColor(vg, nvgRGBA(200, 200, 200, 255));
			nvgStrokeWidth(vg, 2.0f);
			nvgStroke(vg);
		} else if (id == hoveredId) {
			nvgFillColor(vg, nvgRGBA(60, 60, 60, 255));
			nvgFill(vg);
			nvgStrokeColor(vg, nvgRGBA(100, 100, 100, 255));
			nvgStrokeWidth(vg, 1.0f);
			nvgStroke(vg);
		} else {
			nvgFillColor(vg, nvgRGBA(50, 50, 50, 255));
			nvgFill(vg);
			nvgStrokeColor(vg, nvgRGBA(60, 60, 60, 255));
			nvgStrokeWidth(vg, 1.0f);
			nvgStroke(vg);
		}

		// Icon
		int tex = GetTextureForId(id);
		if (tex > 0) {
			int tw, th;
			nvgImageSize(vg, tex, &tw, &th);
			float scale = 64.0f / (float)std::max(tw, th);
			float dw = (float)tw * scale;
			float dh = (float)th * scale;
			NVGpaint imgPaint = nvgImagePattern(vg, r.x + (r.width - dw) / 2.0f, r.y + 6.0f, dw, dh, 0.0f, tex, 1.0f);
			nvgBeginPath(vg);
			nvgRect(vg, r.x + (r.width - dw) / 2.0f, r.y + 6.0f, dw, dh);
			nvgFillPaint(vg, imgPaint);
			nvgFill(vg);
		}

		// Text
		nvgFontSize(vg, 12.0f);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));

		wxString label;
		auto itName = m_nameOverrides.find(id);
		if (itName != m_nameOverrides.end()) {
			label = itName->second;
		} else {
			label = g_items.getItemType(id).name;
		}

		if (label.Length() > 14) {
			label = label.Left(12) + "..";
		}
		nvgText(vg, r.x + r.width / 2.0f, (float)r.y + 75.0f, label.mb_str(), nullptr);

		if (m_showDetails) {
			nvgFontSize(vg, 10.0f);
			nvgFillColor(vg, nvgRGBA(180, 180, 180, 255));
			nvgText(vg, r.x + r.width / 2.0f, (float)r.y + 95.0f, wxString(std::format("S: {}", id)).mb_str(), nullptr);
			nvgFillColor(vg, nvgRGBA(150, 150, 150, 255));
			nvgText(vg, r.x + r.width / 2.0f, (float)r.y + 110.0f, wxString(std::format("C: {}", g_items.getItemType(id).clientID)).mb_str(), nullptr);
		}
	}
}

void ItemGridPanel::OnSize(wxSizeEvent& event) {
	UpdateVirtualSize();
	Refresh();
	event.Skip();
}

wxRect ItemGridPanel::GetItemRect(int index) const {
	int row = index / m_cols;
	int col = index % m_cols;
	return wxRect(padding + col * (item_width + padding), padding + row * (item_height + padding), item_width, item_height);
}

int ItemGridPanel::HitTest(int x, int y) const {
	int ly = y + GetScrollPosition();
	int col = (x - padding) / (item_width + padding);
	int row = (ly - padding) / (item_height + padding);
	if (col < 0 || col >= m_cols || row < 0) {
		return -1;
	}
	int index = row * m_cols + col;
	if (index >= 0 && index < (int)filteredItems.size()) {
		if (GetItemRect(index).Contains(x, ly)) { // GetItemRect is 0-indexed pos. Contains checks against that.
			// No wait, GetItemRect returns pos assuming scroll=0.
			// ly is adjusted for scroll.
			// So if scroll=100_px. Click at 10_px on screen -> ly=110.
			// Row 0 is at 0..80. ly=110 is Row 1.
			// GetItemRect(index) -> returns y=80..160 for Row 1.
			// Contains(x, 110) -> True.
			// Correct.
			return index;
		}
	}
	return -1;
}

void ItemGridPanel::OnMotion(wxMouseEvent& event) {
	int idx = HitTest(event.GetX(), event.GetY());
	uint16_t id = (idx != -1) ? filteredItems[idx] : 0;
	if (hoveredId != id) {
		hoveredId = id;
		Refresh();
	}
	if (idx != -1) {
		SetCursor(wxCursor(wxCURSOR_HAND));
	} else {
		SetCursor(wxNullCursor);
	}

	if (event.Dragging() && m_draggable && selectedId != 0) {
		wxTextDataObject data(wxString(std::format("RME_ITEM:{}", selectedId)));
		wxDropSource dragSource(this);
		dragSource.SetData(data);
		dragSource.DoDragDrop(wxDrag_AllowMove);
	}
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
}

void ItemGridPanel::OnTimer(wxTimerEvent&) { }
void ItemGridPanel::SetOverrideNames(const std::map<uint16_t, wxString>& names) {
	m_nameOverrides = names;
	Refresh();
}
void ItemGridPanel::SetShowDetails(bool show) {
	m_showDetails = show;
	Refresh();
}
void ItemGridPanel::SetDraggable(bool check) {
	m_draggable = check;
}
wxSize ItemGridPanel::DoGetBestClientSize() const {
	return wxSize(FromDIP(240), FromDIP(300));
}
