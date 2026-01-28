//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "app/settings.h"
#include <spdlog/spdlog.h>
#include "ui/gui.h"
#include "brushes/brush.h"
#include "rendering/ui/map_display.h"

#include "palette/palette_window.h"
#include "palette/panels/brush_palette_panel.h"
#include "palette/palette_house.h"
#include "palette/palette_creature.h"
#include "palette/palette_waypoints.h"

// Removed includes for size/tool panels as they are no longer managed here

#include "brushes/house/house_brush.h"
#include "map/map.h"

// ============================================================================
// Palette window

BEGIN_EVENT_TABLE(PaletteWindow, wxPanel)
EVT_CHOICEBOOK_PAGE_CHANGING(PALETTE_CHOICEBOOK, PaletteWindow::OnSwitchingPage)
EVT_CHOICEBOOK_PAGE_CHANGED(PALETTE_CHOICEBOOK, PaletteWindow::OnPageChanged)
EVT_CLOSE(PaletteWindow::OnClose)

EVT_KEY_DOWN(PaletteWindow::OnKey)
END_EVENT_TABLE()

PaletteWindow::PaletteWindow(wxWindow* parent, const TilesetContainer& tilesets) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(230, 250)),
	choicebook(nullptr),
	terrain_palette(nullptr),
	doodad_palette(nullptr),
	item_palette(nullptr),
	collection_palette(nullptr),
	creature_palette(nullptr),
	house_palette(nullptr),
	waypoint_palette(nullptr),
	raw_palette(nullptr) {
	SetMinSize(wxSize(225, 250));

	// Create choicebook
	choicebook = newd wxChoicebook(this, PALETTE_CHOICEBOOK, wxDefaultPosition, wxSize(230, 250));

	terrain_palette = static_cast<BrushPalettePanel*>(CreateTerrainPalette(choicebook, tilesets));
	choicebook->AddPage(terrain_palette, terrain_palette->GetName());

	doodad_palette = static_cast<BrushPalettePanel*>(CreateDoodadPalette(choicebook, tilesets));
	choicebook->AddPage(doodad_palette, doodad_palette->GetName());

	collection_palette = static_cast<BrushPalettePanel*>(CreateCollectionPalette(choicebook, tilesets));
	choicebook->AddPage(collection_palette, collection_palette->GetName());

	item_palette = static_cast<BrushPalettePanel*>(CreateItemPalette(choicebook, tilesets));
	choicebook->AddPage(item_palette, item_palette->GetName());

	house_palette = static_cast<HousePalettePanel*>(CreateHousePalette(choicebook, tilesets));
	choicebook->AddPage(house_palette, house_palette->GetName());

	waypoint_palette = static_cast<WaypointPalettePanel*>(CreateWaypointPalette(choicebook, tilesets));
	choicebook->AddPage(waypoint_palette, waypoint_palette->GetName());

	creature_palette = static_cast<CreaturePalettePanel*>(CreateCreaturePalette(choicebook, tilesets));
	choicebook->AddPage(creature_palette, creature_palette->GetName());

	raw_palette = static_cast<BrushPalettePanel*>(CreateRAWPalette(choicebook, tilesets));
	choicebook->AddPage(raw_palette, raw_palette->GetName());

	// Setup sizers
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	choicebook->SetMinSize(wxSize(225, 300));
	sizer->Add(choicebook, 1, wxEXPAND);
	SetSizer(sizer);

	// Load first page
	LoadCurrentContents();

	Fit();
}

PaletteWindow::~PaletteWindow() {
	////
}

PalettePanel* PaletteWindow::CreateTerrainPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_TERRAIN);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE)));
	return panel;
}

PalettePanel* PaletteWindow::CreateCollectionPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_COLLECTION);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_COLLECTION_STYLE)));
	return panel;
}

PalettePanel* PaletteWindow::CreateDoodadPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_DOODAD);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE)));
	return panel;
}

PalettePanel* PaletteWindow::CreateItemPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_ITEM);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE)));
	return panel;
}

PalettePanel* PaletteWindow::CreateHousePalette(wxWindow* parent, const TilesetContainer& tilesets) {
	HousePalettePanel* panel = newd HousePalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateWaypointPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	WaypointPalettePanel* panel = newd WaypointPalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateCreaturePalette(wxWindow* parent, const TilesetContainer& tilesets) {
	CreaturePalettePanel* panel = newd CreaturePalettePanel(parent);
	return panel;
}

PalettePanel* PaletteWindow::CreateRAWPalette(wxWindow* parent, const TilesetContainer& tilesets) {
	BrushPalettePanel* panel = newd BrushPalettePanel(parent, tilesets, TILESET_RAW);
	panel->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE)));
	return panel;
}

void PaletteWindow::ReloadSettings(Map* map) {
	if (terrain_palette) {
		terrain_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_TERRAIN_STYLE)));
	}
	if (doodad_palette) {
		doodad_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_DOODAD_STYLE)));
	}
	if (house_palette) {
		house_palette->SetMap(map);
	}
	if (waypoint_palette) {
		waypoint_palette->SetMap(map);
	}
	if (item_palette) {
		item_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_ITEM_STYLE)));
	}
	if (collection_palette) {
		collection_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_COLLECTION_STYLE)));
	}
	if (raw_palette) {
		raw_palette->SetListType(wxstr(g_settings.getString(Config::PALETTE_RAW_STYLE)));
	}
	InvalidateContents();
}

void PaletteWindow::LoadCurrentContents() {
	if (!choicebook) {
		return;
	}
	PalettePanel* panel = dynamic_cast<PalettePanel*>(choicebook->GetCurrentPage());
	panel->LoadCurrentContents();
	Fit();
	Refresh();
	Update();
}

void PaletteWindow::InvalidateContents() {
	if (!choicebook) {
		return;
	}
	for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
		PalettePanel* panel = dynamic_cast<PalettePanel*>(choicebook->GetPage(iz));
		panel->InvalidateContents();
	}
	LoadCurrentContents();
	if (creature_palette) {
		creature_palette->OnUpdate();
	}
	if (house_palette) {
		house_palette->OnUpdate();
	}
	if (waypoint_palette) {
		waypoint_palette->OnUpdate();
	}
}

void PaletteWindow::SelectPage(PaletteType id) {
	if (!choicebook) {
		return;
	}
	if (id == GetSelectedPage()) {
		return;
	}

	for (size_t iz = 0; iz < choicebook->GetPageCount(); ++iz) {
		PalettePanel* panel = dynamic_cast<PalettePanel*>(choicebook->GetPage(iz));
		if (panel->GetType() == id) {
			choicebook->SetSelection(iz);
			// LoadCurrentContents();
			break;
		}
	}
}

Brush* PaletteWindow::GetSelectedBrush() const {
	if (!choicebook) {
		return nullptr;
	}
	PalettePanel* panel = dynamic_cast<PalettePanel*>(choicebook->GetCurrentPage());
	return panel->GetSelectedBrush();
}

int PaletteWindow::GetSelectedBrushSize() const {
	if (!choicebook) {
		return 0;
	}
	PalettePanel* panel = dynamic_cast<PalettePanel*>(choicebook->GetCurrentPage());
	return panel->GetSelectedBrushSize();
}

PaletteType PaletteWindow::GetSelectedPage() const {
	if (!choicebook) {
		return TILESET_UNKNOWN;
	}
	PalettePanel* panel = dynamic_cast<PalettePanel*>(choicebook->GetCurrentPage());
	if (!panel) {
		return TILESET_UNKNOWN;
	}
	return panel->GetType();
}

bool PaletteWindow::OnSelectBrush(const Brush* whatbrush, PaletteType primary) {
	if (!choicebook || !whatbrush) {
		return false;
	}

	if (whatbrush->isHouse() && house_palette) {
		house_palette->SelectBrush(whatbrush);
		SelectPage(TILESET_HOUSE);
		return true;
	}

	switch (primary) {
		case TILESET_TERRAIN: {
			// This is already searched first
			break;
		}
		case TILESET_DOODAD: {
			// Ok, search doodad before terrain
			if (doodad_palette && doodad_palette->SelectBrush(whatbrush)) {
				SelectPage(TILESET_DOODAD);
				return true;
			}
			break;
		}
		case TILESET_COLLECTION: {
			if (collection_palette && collection_palette->SelectBrush(whatbrush)) {
				SelectPage(TILESET_COLLECTION);
				return true;
			}
		}
		case TILESET_ITEM: {
			if (item_palette && item_palette->SelectBrush(whatbrush)) {
				SelectPage(TILESET_ITEM);
				return true;
			}
			break;
		}
		case TILESET_CREATURE: {
			if (creature_palette && creature_palette->SelectBrush(whatbrush)) {
				SelectPage(TILESET_CREATURE);
				return true;
			}
			break;
		}
		case TILESET_RAW: {
			if (raw_palette && raw_palette->SelectBrush(whatbrush)) {
				SelectPage(TILESET_RAW);
				return true;
			}
			break;
		}
		default:
			break;
	}

	// Test if it's a terrain brush
	if (terrain_palette && terrain_palette->SelectBrush(whatbrush)) {
		SelectPage(TILESET_TERRAIN);
		return true;
	}

	// Test if it's a doodad brush
	if (primary != TILESET_DOODAD) {
		if (doodad_palette && doodad_palette->SelectBrush(whatbrush)) {
			SelectPage(TILESET_DOODAD);
			return true;
		}
	}

	// Test if it's an item brush
	if (primary != TILESET_ITEM) {
		if (item_palette && item_palette->SelectBrush(whatbrush)) {
			SelectPage(TILESET_ITEM);
			return true;
		}
	}

	// Test if it's a creature brush
	if (primary != TILESET_CREATURE) {
		if (creature_palette && creature_palette->SelectBrush(whatbrush)) {
			SelectPage(TILESET_CREATURE);
			return true;
		}
	}

	// Test if it's a raw brush
	if (primary != TILESET_RAW) {
		if (raw_palette && raw_palette->SelectBrush(whatbrush)) {
			SelectPage(TILESET_RAW);
			return true;
		}
	}

	return false;
}

void PaletteWindow::OnSwitchingPage(wxChoicebookEvent& event) {
	event.Skip();
	if (!choicebook) {
		return;
	}

	wxWindow* old_page = choicebook->GetPage(choicebook->GetSelection());
	PalettePanel* old_panel = dynamic_cast<PalettePanel*>(old_page);
	if (old_panel) {
		old_panel->OnSwitchOut();
	}

	wxWindow* page = choicebook->GetPage(event.GetSelection());
	PalettePanel* panel = dynamic_cast<PalettePanel*>(page);
	if (panel) {
		panel->OnSwitchIn();
	}
}

void PaletteWindow::OnPageChanged(wxChoicebookEvent& event) {
	if (!choicebook) {
		return;
	}
	g_gui.ActivatePalette(this);
	g_gui.SelectBrush();
}

void PaletteWindow::OnUpdateBrushSize(BrushShape shape, int size) {
	if (!choicebook) {
		return;
	}
	PalettePanel* page = dynamic_cast<PalettePanel*>(choicebook->GetCurrentPage());
	ASSERT(page);
	page->OnUpdateBrushSize(shape, size);
}

void PaletteWindow::OnUpdate(Map* map) {
	if (creature_palette) {
		creature_palette->OnUpdate();
	}
	if (house_palette) {
		house_palette->SetMap(map);
	}
	if (waypoint_palette) {
		waypoint_palette->SetMap(map);
		waypoint_palette->OnUpdate();
	}
}

void PaletteWindow::OnKey(wxKeyEvent& event) {
	if (g_gui.GetCurrentTab() != nullptr) {
		g_gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
	}
}

void PaletteWindow::OnClose(wxCloseEvent& event) {
	if (!event.CanVeto()) {
		// We can't do anything! This sucks!
		// (application is closed, we have to destroy ourselves)
		Destroy();
	} else {
		Show(false);
		event.Veto(true);
	}
}
