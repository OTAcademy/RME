#include "ui/tool_options_window.h"
#include "ui/tool_options_surface.h"
#include "app/settings.h"
#include "ui/gui.h"

ToolOptionsWindow::ToolOptionsWindow(wxWindow* parent) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize) {

	main_sizer = newd wxBoxSizer(wxVERTICAL);

	// The new unified surface
	surface = newd ToolOptionsSurface(this);
	main_sizer->Add(surface, 1, wxEXPAND | wxALL, 0);

	SetSizer(main_sizer);

	// Initialize settings
	ReloadSettings();
}

ToolOptionsWindow::~ToolOptionsWindow() {
	// Children deleted automatically
}

void ToolOptionsWindow::SetPaletteType(PaletteType type) {
	if (surface) {
		surface->SetPaletteType(type);
		Layout(); // Re-trigger layout in case best size changed
		GetParent()->Layout(); // Inform parent (AUI pane)
	}
}

void ToolOptionsWindow::UpdateBrushSize(BrushShape shape, int size) {
	if (surface) {
		surface->UpdateBrushSize(shape, size);
	}
}

void ToolOptionsWindow::ReloadSettings() {
	if (surface) {
		surface->ReloadSettings();
	}
}
