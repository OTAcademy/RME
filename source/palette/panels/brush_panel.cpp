#include "palette/panels/brush_panel.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "app/settings.h"
#include "palette/palette_window.h" // For PaletteWindow dynamic_casts
#include "palette/controls/virtual_brush_grid.h"
#include <spdlog/spdlog.h>
#include <wx/wrapsizer.h>
#include <algorithm>
#include <iterator>

// ============================================================================
// Brush Panel
// A container of brush buttons

BrushPanel::BrushPanel(wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	tileset(nullptr),
	brushbox(nullptr),
	loaded(false),
	list_type(BRUSHLIST_LISTBOX) {
	sizer = newd wxBoxSizer(wxVERTICAL);
	SetSizerAndFit(sizer);

	Bind(wxEVT_LISTBOX, &BrushPanel::OnClickListBoxRow, this, wxID_ANY);
}

BrushPanel::~BrushPanel() {
	////
}

void BrushPanel::AssignTileset(const TilesetCategory* _tileset) {
	if (_tileset != tileset) {
		InvalidateContents();
		tileset = _tileset;
	}
}

void BrushPanel::SetListType(BrushListType ltype) {
	if (list_type != ltype) {
		InvalidateContents();
		list_type = ltype;
	}
}

void BrushPanel::SetListType(wxString ltype) {
	if (ltype == "small icons") {
		SetListType(BRUSHLIST_SMALL_ICONS);
	} else if (ltype == "large icons") {
		SetListType(BRUSHLIST_LARGE_ICONS);
	} else if (ltype == "listbox") {
		SetListType(BRUSHLIST_LISTBOX);
	} else if (ltype == "textlistbox") {
		SetListType(BRUSHLIST_TEXT_LISTBOX);
	}
}

void BrushPanel::InvalidateContents() {
	sizer->Clear(true);
	loaded = false;
	brushbox = nullptr;
}

void BrushPanel::LoadContents() {
	if (loaded) {
		return;
	}

	ASSERT(tileset != nullptr);

	switch (list_type) {
		case BRUSHLIST_LARGE_ICONS:
			brushbox = newd VirtualBrushGrid(this, tileset, RENDER_SIZE_32x32);
			break;
		case BRUSHLIST_SMALL_ICONS:
			brushbox = newd VirtualBrushGrid(this, tileset, RENDER_SIZE_16x16);
			break;
		case BRUSHLIST_LISTBOX:
		case BRUSHLIST_TEXT_LISTBOX:
			brushbox = newd BrushListBox(this, tileset);
			break;
		default:
			break;
	}

	if (!brushbox) {
		return;
	}

	loaded = true;
	sizer->Add(brushbox->GetSelfWindow(), 1, wxEXPAND);
	Layout();
	Fit();
	brushbox->SelectFirstBrush();
}

void BrushPanel::SelectFirstBrush() {
	if (loaded) {
		ASSERT(brushbox != nullptr);
		brushbox->SelectFirstBrush();
	}
}

Brush* BrushPanel::GetSelectedBrush() const {
	if (loaded) {
		ASSERT(brushbox != nullptr);
		return brushbox->GetSelectedBrush();
	}

	if (tileset && tileset->size() > 0) {
		return tileset->brushlist[0];
	}
	return nullptr;
}

bool BrushPanel::SelectBrush(const Brush* whatbrush) {
	if (loaded) {
		ASSERT(brushbox != nullptr);
		return brushbox->SelectBrush(whatbrush);
	}

	for (const auto* brush : tileset->brushlist) {
		if (brush == whatbrush) {
			LoadContents();
			return brushbox->SelectBrush(whatbrush);
		}
	}
	return false;
}

void BrushPanel::OnSwitchIn() {
	spdlog::info("BrushPanel::OnSwitchIn");
	LoadContents();
}

void BrushPanel::OnSwitchOut() {
	////
}

void BrushPanel::OnClickListBoxRow(wxCommandEvent& event) {
	ASSERT(tileset->getType() >= TILESET_UNKNOWN && tileset->getType() <= TILESET_HOUSE);
	// We just notify the GUI of the action, it will take care of everything else
	ASSERT(brushbox);
	size_t n = event.GetSelection();

	wxWindow* w = this->GetParent();
	while (w) {
		PaletteWindow* pw = dynamic_cast<PaletteWindow*>(w);
		if (pw) {
			g_gui.ActivatePalette(pw);
			break;
		}
		w = w->GetParent();
	}

	g_gui.SelectBrush(tileset->brushlist[n], tileset->getType());
}


// ============================================================================
// BrushListBox

BrushListBox::BrushListBox(wxWindow* parent, const TilesetCategory* tileset) :
	wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE),
	BrushBoxInterface(tileset) {
	SetItemCount(tileset->size());
	Bind(wxEVT_KEY_DOWN, &BrushListBox::OnKey, this);
}

BrushListBox::~BrushListBox() {
	////
}

void BrushListBox::SelectFirstBrush() {
	SetSelection(0);
	wxWindow::ScrollLines(-1);
}

Brush* BrushListBox::GetSelectedBrush() const {
	if (!tileset) {
		return nullptr;
	}

	int n = GetSelection();
	if (n != wxNOT_FOUND) {
		return tileset->brushlist[n];
	} else if (tileset->size() > 0) {
		return tileset->brushlist[0];
	}
	return nullptr;
}

bool BrushListBox::SelectBrush(const Brush* whatbrush) {
	auto it = std::ranges::find(tileset->brushlist, whatbrush);
	if (it != tileset->brushlist.end()) {
		SetSelection(std::distance(tileset->brushlist.begin(), it));
		return true;
	}
	return false;
}

void BrushListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const {
	ASSERT(n < tileset->size());
	Sprite* spr = tileset->brushlist[n]->getSprite();
	if (!spr) {
		spr = g_gui.gfx.getSprite(tileset->brushlist[n]->getLookID());
	}
	if (spr) {
		spr->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}
	if (IsSelected(n)) {
		if (HasFocus()) {
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		} else {
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
		}
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}
	dc.DrawText(wxstr(tileset->brushlist[n]->getName()), rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord BrushListBox::OnMeasureItem(size_t n) const {
	return 32;
}

void BrushListBox::OnKey(wxKeyEvent& event) {
	switch (event.GetKeyCode()) {
		case WXK_UP:
		case WXK_DOWN:
		case WXK_LEFT:
		case WXK_RIGHT:
		case WXK_PAGEUP:
		case WXK_PAGEDOWN:
		case WXK_HOME:
		case WXK_END:
			if (g_settings.getInteger(Config::LISTBOX_EATS_ALL_EVENTS)) {
				event.Skip(true);
			} else {
				if (g_gui.GetCurrentTab() != nullptr) {
					g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
				}
			}
			break;
		default:
			if (g_gui.GetCurrentTab() != nullptr) {
				g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
			}
			break;
	}
}
