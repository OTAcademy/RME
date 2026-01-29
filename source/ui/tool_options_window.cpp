#include "ui/tool_options_window.h"
#include "palette/panels/brush_size_panel.h"
#include "palette/panels/brush_tool_panel.h"
#include "palette/panels/brush_thickness_panel.h"
#include "app/settings.h"
#include <spdlog/spdlog.h>

ToolOptionsWindow::ToolOptionsWindow(wxWindow* parent) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize), // wxTAB_TRAVERSAL?
	size_panel(nullptr),
	tool_panel(nullptr),
	thickness_panel(nullptr),
	current_type(TILESET_UNKNOWN) {

	main_sizer = newd wxBoxSizer(wxVERTICAL);

	// Create panels
	// We pass 'this' as parent. They will be children of ToolOptionsWindow.
	tool_panel = newd BrushToolPanel(this);
	thickness_panel = newd BrushThicknessPanel(this);
	size_panel = newd BrushSizePanel(this);

	// Order matters for display? usually Tools then Size?
	// In old palette: it depended on creation order.
	// Usually: Tools, then Thickness, then Size.

	main_sizer->Add(tool_panel, 0, wxEXPAND | wxALL, 2);
	main_sizer->Add(thickness_panel, 0, wxEXPAND | wxALL, 2);
	main_sizer->Add(size_panel, 0, wxEXPAND | wxALL, 2);

	SetSizer(main_sizer);

	// Initial state: hide all? or default?
	// SetPaletteType(TILESET_TERRAIN); // Intention: Don't load yet, waits for ActivatePalette
}

ToolOptionsWindow::~ToolOptionsWindow() {
	// Children deleted automatically
}

void ToolOptionsWindow::SetPaletteType(PaletteType type) {
	if (current_type == type) {
		return;
	}
	current_type = type;

	// Logic to show/hide panels based on type
	bool show_tools = false;
	bool show_thickness = false;
	bool show_size = false;

	switch (type) {
		case TILESET_TERRAIN:
		case TILESET_COLLECTION: // Collections had tools and size in old code (and thickness!)
			show_tools = true;
			show_size = true;
			if (type == TILESET_COLLECTION) {
				show_thickness = true;
			}
			break;
		case TILESET_DOODAD:
			show_thickness = true;
			show_size = true;
			break;
		case TILESET_ITEM:
		case TILESET_HOUSE:
		case TILESET_RAW:
			show_size = true;
			break;
		default:
			break;
	}

	// Apply visibility
	tool_panel->Show(show_tools);
	thickness_panel->Show(show_thickness);
	size_panel->Show(show_size);

	// Configure specific settings for the palette type if needed
	// E.g. icon sizes
	if (show_tools) {
		bool large = false;
		if (type == TILESET_TERRAIN) {
			large = g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR);
		}
		if (type == TILESET_COLLECTION) {
			large = g_settings.getBoolean(Config::USE_LARGE_COLLECTION_TOOLBAR);
		}
		tool_panel->SetToolbarIconSize(large);
		tool_panel->OnSwitchIn();
	}

	if (show_size) {
		bool large = false;
		if (type == TILESET_TERRAIN) {
			large = g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR);
		} else if (type == TILESET_DOODAD) {
			large = g_settings.getBoolean(Config::USE_LARGE_DOODAD_SIZEBAR);
		} else if (type == TILESET_ITEM) {
			large = g_settings.getBoolean(Config::USE_LARGE_ITEM_SIZEBAR);
		} else if (type == TILESET_HOUSE) {
			large = g_settings.getBoolean(Config::USE_LARGE_HOUSE_SIZEBAR);
		} else if (type == TILESET_RAW) {
			large = g_settings.getBoolean(Config::USE_LARGE_RAW_SIZEBAR);
		} else if (type == TILESET_COLLECTION) {
			large = g_settings.getBoolean(Config::USE_LARGE_COLLECTION_TOOLBAR);
		}

		size_panel->SetToolbarIconSize(large);
		size_panel->OnSwitchIn();
	}

	if (show_thickness) {
		thickness_panel->OnSwitchIn();
	}

	Layout();
	Fit();
	Refresh();
}

void ToolOptionsWindow::UpdateBrushSize(BrushShape shape, int size) {
	if (size_panel && size_panel->IsShown()) {
		size_panel->OnUpdateBrushSize(shape, size);
	}
}

void ToolOptionsWindow::ReloadSettings() {
	// Re-apply settings based on current type
	PaletteType t = current_type;
	current_type = TILESET_UNKNOWN; // Force refresh
	SetPaletteType(t);
}
