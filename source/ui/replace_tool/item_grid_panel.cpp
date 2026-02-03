#include "ui/replace_tool/item_grid_panel.h"
#include "ui/theme.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include <glad/glad.h>
#define NANOVG_GL3
#include <nanovg.h>
#include <nanovg_gl.h>
#include <wx/dcclient.h>
#include <wx/graphics.h>
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <algorithm>
#include <iostream>

ItemGridPanel::ItemGridPanel(wxWindow* parent, Listener* listener) :
	wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxWANTS_CHARS),
	listener(listener), m_animTimer(this) {

	SetBackgroundStyle(wxBG_STYLE_PAINT);

	Bind(wxEVT_PAINT, &ItemGridPanel::OnPaint, this);
	Bind(wxEVT_SIZE, &ItemGridPanel::OnSize, this);
	Bind(wxEVT_TIMER, &ItemGridPanel::OnTimer, this);
	Bind(wxEVT_LEFT_DOWN, &ItemGridPanel::OnMouse, this);
	Bind(wxEVT_MOTION, &ItemGridPanel::OnMotion, this);
	Bind(wxEVT_LEAVE_WINDOW, &ItemGridPanel::OnMouse, this);
	Bind(wxEVT_MOUSEWHEEL, &ItemGridPanel::OnMouseWheel, this);

	Bind(wxEVT_ERASE_BACKGROUND, [](wxEraseEvent&) { /* No-op */ });
}

ItemGridPanel::~ItemGridPanel() {
	if (m_glContext) {
		SetCurrent(*m_glContext);
		ClearTextureCache();
		if (m_nvg) {
			nvgDeleteGL3(m_nvg);
		}
	}
	delete m_glContext;
}

void ItemGridPanel::InitGL() {
	if (m_glContext) {
		return;
	}

	m_glContext = new wxGLContext(this);
	SetCurrent(*m_glContext);

	if (!gladLoadGL()) {
		return;
	}

	m_nvg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
	if (m_nvg) {
		// Load font (same as TextRenderer)
		nvgCreateFont(m_nvg, "sans", "C:\\Windows\\Fonts\\arial.ttf");
	}
}

void ItemGridPanel::ClearTextureCache() {
	if (!m_nvg) {
		return;
	}
	for (const auto& [id, tex] : m_textureCache) {
		nvgDeleteImage(m_nvg, tex);
	}
	m_textureCache.clear();
}

int ItemGridPanel::GetTextureForId(uint16_t id) {
	if (m_textureCache.count(id)) {
		return m_textureCache[id];
	}

	const ItemType& it = g_items.getItemType(id);
	GameSprite* gs = static_cast<GameSprite*>(g_gui.gfx.getSprite(it.clientID));
	if (!gs) {
		return 0;
	}

	int w = gs->width * 32;
	int h = gs->height * 32;
	if (w <= 0 || h <= 0) {
		return 0;
	}

	size_t bufferSize = static_cast<size_t>(w) * h * 4;
	std::vector<uint8_t> composite(bufferSize, 0);

	int px = (gs->pattern_x >= 3) ? 2 : 0;
	for (int l = 0; l < gs->layers; ++l) {
		for (int sw = 0; sw < gs->width; ++sw) {
			for (int sh = 0; sh < gs->height; ++sh) {
				int idx = gs->getIndex(sw, sh, l, px, 0, 0, 0);
				if (idx < 0 || (size_t)idx >= gs->spriteList.size()) {
					continue;
				}

				auto data = gs->spriteList[idx]->getRGBAData();
				if (!data) {
					continue;
				}

				int part_x = (gs->width - sw - 1) * 32;
				int part_y = (gs->height - sh - 1) * 32;

				for (int sy = 0; sy < 32; ++sy) {
					for (int sx = 0; sx < 32; ++sx) {
						int dy = part_y + sy;
						int dx = part_x + sx;
						int di = (dy * w + dx) * 4;
						int si = (sy * 32 + sx) * 4;

						uint8_t sa = data[si + 3];
						if (sa == 0) {
							continue;
						}

						if (sa == 255) {
							composite[di + 0] = data[si + 0];
							composite[di + 1] = data[si + 1];
							composite[di + 2] = data[si + 2];
							composite[di + 3] = 255;
						} else {
							float a = sa / 255.0f;
							float ia = 1.0f - a;
							composite[di + 0] = (uint8_t)(data[si + 0] * a + composite[di + 0] * ia);
							composite[di + 1] = (uint8_t)(data[si + 1] * a + composite[di + 1] * ia);
							composite[di + 2] = (uint8_t)(data[si + 2] * a + composite[di + 2] * ia);
							composite[di + 3] = std::max(composite[di + 3], sa);
						}
					}
				}
				gs->unloadDC();
			}
		}
	}

	int tex = nvgCreateImageRGBA(m_nvg, w, h, 0, composite.data());
	m_textureCache[id] = tex;
	return tex;
}

void ItemGridPanel::SetItems(const std::vector<uint16_t>& items) {
	allItems = items;
	m_nameOverrides.clear();
	if (m_glContext) {
		SetCurrent(*m_glContext);
		ClearTextureCache();
	}
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
			if (name.Lower().Contains(lowerFilter) || wxString::Format("%d", id).Contains(lowerFilter) || wxString::Format("%d", it.clientID).Contains(lowerFilter)) {
				filteredItems.push_back(id);
			}
		}
	}
	m_scrollPos = 0;
	UpdateVirtualSize();
	Refresh();
}

void ItemGridPanel::UpdateVirtualSize() {
	int width, height;
	GetClientSize(&width, &height);
	m_cols = std::max(1, (width - padding) / (item_width + padding));
	m_maxRows = (int)((filteredItems.size() + m_cols - 1) / m_cols);

	// Range is the total height in pixels
	int totalHeight = m_maxRows * (item_height + padding);
	// Thumb is the visible height
	// We use rows as units for simple scrollbar logic if we want, but pixel-based is fine too.
	SetScrollbar(wxVERTICAL, m_scrollPos, height, totalHeight);
}

void ItemGridPanel::OnPaint(wxPaintEvent&) {
	InitGL();
	if (!m_nvg) {
		return;
	}

	SetCurrent(*m_glContext);

	int w, h;
	GetClientSize(&w, &h);
	glViewport(0, 0, w, h);
	glClearColor(45 / 255.0f, 45 / 255.0f, 45 / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	nvgBeginFrame(m_nvg, w, h, 1.0f);
	nvgSave(m_nvg);
	nvgTranslate(m_nvg, 0, -m_scrollPos);

	int rowH = item_height + padding;
	int startRow = m_scrollPos / rowH;
	int endRow = startRow + (h / rowH) + 2;

	size_t startIdx = (size_t)startRow * m_cols;
	size_t endIdx = std::min(filteredItems.size(), (size_t)endRow * m_cols);

	for (size_t i = startIdx; i < endIdx; ++i) {
		uint16_t id = filteredItems[i];
		wxRect r = GetItemRect(i);

		// Card
		nvgBeginPath(m_nvg);
		nvgRoundedRect(m_nvg, r.x, r.y, r.width, r.height, 4.0f);
		if (id == selectedId) {
			nvgFillColor(m_nvg, nvgRGBA(80, 80, 80, 255));
			nvgFill(m_nvg);
			nvgStrokeColor(m_nvg, nvgRGBA(200, 200, 200, 255));
			nvgStrokeWidth(m_nvg, 2.0f);
			nvgStroke(m_nvg);
		} else if (id == hoveredId) {
			nvgFillColor(m_nvg, nvgRGBA(60, 60, 60, 255));
			nvgFill(m_nvg);
			nvgStrokeColor(m_nvg, nvgRGBA(100, 100, 100, 255));
			nvgStrokeWidth(m_nvg, 1.0f);
			nvgStroke(m_nvg);
		} else {
			nvgFillColor(m_nvg, nvgRGBA(50, 50, 50, 255));
			nvgFill(m_nvg);
			nvgStrokeColor(m_nvg, nvgRGBA(60, 60, 60, 255));
			nvgStrokeWidth(m_nvg, 1.0f);
			nvgStroke(m_nvg);
		}

		// Icon
		int tex = GetTextureForId(id);
		if (tex > 0) {
			int tw, th;
			nvgImageSize(m_nvg, tex, &tw, &th);
			float scale = 64.0f / (float)std::max(tw, th);
			float dw = (float)tw * scale;
			float dh = (float)th * scale;
			NVGpaint imgPaint = nvgImagePattern(m_nvg, r.x + (r.width - dw) / 2.0f, r.y + 6.0f, dw, dh, 0.0f, tex, 1.0f);
			nvgBeginPath(m_nvg);
			nvgRect(m_nvg, r.x + (r.width - dw) / 2.0f, r.y + 6.0f, dw, dh);
			nvgFillPaint(m_nvg, imgPaint);
			nvgFill(m_nvg);
		}

		// Text
		nvgFontSize(m_nvg, 12.0f);
		nvgFontFace(m_nvg, "sans");
		nvgTextAlign(m_nvg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		nvgFillColor(m_nvg, nvgRGBA(255, 255, 255, 255));

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
		nvgText(m_nvg, r.x + r.width / 2.0f, (float)r.y + 75.0f, label.mb_str(), nullptr);

		if (m_showDetails) {
			nvgFontSize(m_nvg, 10.0f);
			nvgFillColor(m_nvg, nvgRGBA(180, 180, 180, 255));
			nvgText(m_nvg, r.x + r.width / 2.0f, (float)r.y + 95.0f, wxString::Format("S: %d", id).mb_str(), nullptr);
			nvgFillColor(m_nvg, nvgRGBA(150, 150, 150, 255));
			nvgText(m_nvg, r.x + r.width / 2.0f, (float)r.y + 110.0f, wxString::Format("C: %d", g_items.getItemType(id).clientID).mb_str(), nullptr);
		}
	}

	nvgRestore(m_nvg);
	nvgEndFrame(m_nvg);
	SwapBuffers();
}

void ItemGridPanel::OnMouseWheel(wxMouseEvent& event) {
	int rotation = event.GetWheelRotation();
	m_scrollPos -= (rotation / 120) * (item_height + padding);
	int maxScroll = std::max(0, m_maxRows * (item_height + padding) - GetClientSize().y);
	m_scrollPos = std::max(0, std::min(m_scrollPos, maxScroll));
	UpdateVirtualSize();
	Refresh();
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
	int ly = y + m_scrollPos;
	int col = (x - padding) / (item_width + padding);
	int row = (ly - padding) / (item_height + padding);
	if (col < 0 || col >= m_cols || row < 0) {
		return -1;
	}
	int index = row * m_cols + col;
	if (index >= 0 && index < (int)filteredItems.size()) {
		if (GetItemRect(index).Contains(x, ly)) {
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
		wxTextDataObject data(wxString::Format("RME_ITEM:%d", selectedId));
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
