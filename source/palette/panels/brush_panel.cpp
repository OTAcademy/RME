#include "palette/panels/brush_panel.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "app/settings.h"
#include "palette/palette_window.h" // For PaletteWindow dynamic_casts
#include "palette/controls/virtual_brush_grid.h"
#include <spdlog/spdlog.h>

// ============================================================================
// Brush Panel
// A container of brush buttons

BEGIN_EVENT_TABLE(BrushPanel, wxPanel)
// Listbox style
EVT_LISTBOX(wxID_ANY, BrushPanel::OnClickListBoxRow)
END_EVENT_TABLE()

BrushPanel::BrushPanel(wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	tileset(nullptr),
	brushbox(nullptr),
	loaded(false),
	list_type(BRUSHLIST_LISTBOX) {
	sizer = newd wxBoxSizer(wxVERTICAL);
	SetSizerAndFit(sizer);
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
	loaded = true;
	ASSERT(tileset != nullptr);

	switch (list_type) {
		case BRUSHLIST_LARGE_ICONS:
			brushbox = newd VirtualBrushGrid(this, tileset, RENDER_SIZE_32x32);
			break;
		case BRUSHLIST_SMALL_ICONS:
			brushbox = newd VirtualBrushGrid(this, tileset, RENDER_SIZE_16x16);
			break;
		case BRUSHLIST_LISTBOX:
			brushbox = newd BrushListBox(this, tileset);
			break;
		default:
			break;
	}
	if (!brushbox) {
		return;
	}
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

	for (BrushVector::const_iterator iter = tileset->brushlist.begin(); iter != tileset->brushlist.end(); ++iter) {
		if (*iter == whatbrush) {
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
// BrushIconBox

BEGIN_EVENT_TABLE(BrushIconBox, wxScrolledWindow)
// Listbox style
EVT_TOGGLEBUTTON(wxID_ANY, BrushIconBox::OnClickBrushButton)
END_EVENT_TABLE()

BrushIconBox::BrushIconBox(wxWindow* parent, const TilesetCategory* _tileset, RenderSize rsz) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL),
	BrushBoxInterface(_tileset),
	icon_size(rsz) {
	ASSERT(tileset->getType() >= TILESET_UNKNOWN && tileset->getType() <= TILESET_HOUSE);
	int width;
	if (icon_size == RENDER_SIZE_32x32) {
		width = max(g_settings.getInteger(Config::PALETTE_COL_COUNT) / 2 + 1, 1);
	} else {
		width = max(g_settings.getInteger(Config::PALETTE_COL_COUNT) + 1, 1);
	}

	// Create buttons
	wxSizer* stacksizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* rowsizer = nullptr;
	int item_counter = 0;
	for (BrushVector::const_iterator iter = tileset->brushlist.begin(); iter != tileset->brushlist.end(); ++iter) {
		ASSERT(*iter);
		++item_counter;

		if (!rowsizer) {
			rowsizer = newd wxBoxSizer(wxHORIZONTAL);
		}

		BrushButton* bb = newd BrushButton(this, *iter, rsz);
		rowsizer->Add(bb);
		brush_buttons.push_back(bb);

		if (item_counter % width == 0) { // newd row
			stacksizer->Add(rowsizer);
			rowsizer = nullptr;
		}
	}
	if (rowsizer) {
		stacksizer->Add(rowsizer);
	}

	SetScrollbars(20, 20, 8, item_counter / width, 0, 0);
	SetSizer(stacksizer);
}

BrushIconBox::~BrushIconBox() {
	////
}

void BrushIconBox::SelectFirstBrush() {
	if (tileset && tileset->size() > 0) {
		DeselectAll();
		brush_buttons[0]->SetValue(true);
		EnsureVisible((size_t)0);
	}
}

Brush* BrushIconBox::GetSelectedBrush() const {
	if (!tileset) {
		return nullptr;
	}

	for (std::vector<BrushButton*>::const_iterator it = brush_buttons.begin(); it != brush_buttons.end(); ++it) {
		if ((*it)->GetValue()) {
			return (*it)->brush;
		}
	}
	return nullptr;
}

bool BrushIconBox::SelectBrush(const Brush* whatbrush) {
	DeselectAll();
	for (std::vector<BrushButton*>::iterator it = brush_buttons.begin(); it != brush_buttons.end(); ++it) {
		if ((*it)->brush == whatbrush) {
			(*it)->SetValue(true);
			EnsureVisible(*it);
			return true;
		}
	}
	return false;
}

void BrushIconBox::DeselectAll() {
	for (std::vector<BrushButton*>::iterator it = brush_buttons.begin(); it != brush_buttons.end(); ++it) {
		(*it)->SetValue(false);
	}
}

void BrushIconBox::EnsureVisible(BrushButton* btn) {
	int windowSizeX, windowSizeY;
	GetVirtualSize(&windowSizeX, &windowSizeY);

	int scrollUnitX;
	int scrollUnitY;
	GetScrollPixelsPerUnit(&scrollUnitX, &scrollUnitY);

	wxRect rect = btn->GetRect();
	int y;
	CalcUnscrolledPosition(0, rect.y, nullptr, &y);

	int maxScrollPos = windowSizeY / scrollUnitY;
	int scrollPosY = std::min(maxScrollPos, (y / scrollUnitY));

	int startScrollPosY;
	GetViewStart(nullptr, &startScrollPosY);

	int clientSizeX, clientSizeY;
	GetClientSize(&clientSizeX, &clientSizeY);
	int endScrollPosY = startScrollPosY + clientSizeY / scrollUnitY;

	if (scrollPosY < startScrollPosY || scrollPosY > endScrollPosY) {
		// only scroll if the button isnt visible
		Scroll(-1, scrollPosY);
	}
}

void BrushIconBox::EnsureVisible(size_t n) {
	EnsureVisible(brush_buttons[n]);
}

void BrushIconBox::OnClickBrushButton(wxCommandEvent& event) {
	wxObject* obj = event.GetEventObject();
	BrushButton* btn = dynamic_cast<BrushButton*>(obj);
	if (btn) {
		wxWindow* w = this->GetParent();
		while (w) {
			PaletteWindow* pw = dynamic_cast<PaletteWindow*>(w);
			if (pw) {
				g_gui.ActivatePalette(pw);
				break;
			}
			w = w->GetParent();
		}
		g_gui.SelectBrush(btn->brush, tileset->getType());
	}
}

// ============================================================================
// BrushListBox

BEGIN_EVENT_TABLE(BrushListBox, wxVListBox)
EVT_KEY_DOWN(BrushListBox::OnKey)
END_EVENT_TABLE()

BrushListBox::BrushListBox(wxWindow* parent, const TilesetCategory* tileset) :
	wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE),
	BrushBoxInterface(tileset) {
	SetItemCount(tileset->size());
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
	for (size_t n = 0; n < tileset->size(); ++n) {
		if (tileset->brushlist[n] == whatbrush) {
			SetSelection(n);
			return true;
		}
	}
	return false;
}

void BrushListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const {
	ASSERT(n < tileset->size());
	Sprite* spr = g_gui.gfx.getSprite(tileset->brushlist[n]->getLookID());
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
			if (g_settings.getInteger(Config::LISTBOX_EATS_ALL_EVENTS)) {
				case WXK_PAGEUP:
				case WXK_PAGEDOWN:
				case WXK_HOME:
				case WXK_END:
					event.Skip(true);
			} else {
				[[fallthrough]];
				default:
					if (g_gui.GetCurrentTab() != nullptr) {
						g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
					}
			}
	}
}
