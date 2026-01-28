#include "app/main.h"
#include "palette/panels/brush_thickness_panel.h"
#include "ui/gui.h"
#include "palette/managers/palette_manager.h"
#include "brushes/managers/brush_manager.h"
#include "palette/palette_window.h"
#include <spdlog/spdlog.h>

// ============================================================================
// Brush Thickness Panel

BEGIN_EVENT_TABLE(BrushThicknessPanel, PalettePanel)
#ifdef __WINDOWS__
// This only works in wxmsw
EVT_COMMAND_SCROLL_CHANGED(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
#else
EVT_COMMAND_SCROLL_TOP(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
EVT_COMMAND_SCROLL_BOTTOM(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
EVT_COMMAND_SCROLL_LINEUP(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
EVT_COMMAND_SCROLL_LINEDOWN(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
EVT_COMMAND_SCROLL_PAGEUP(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
EVT_COMMAND_SCROLL_PAGEDOWN(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
EVT_COMMAND_SCROLL_THUMBRELEASE(PALETTE_DOODAD_SLIDER, BrushThicknessPanel::OnScroll)
#endif

EVT_CHECKBOX(PALETTE_DOODAD_USE_THICKNESS, BrushThicknessPanel::OnClickCustomThickness)
END_EVENT_TABLE()

BrushThicknessPanel::BrushThicknessPanel(wxWindow* parent) :
	PalettePanel(parent, wxID_ANY) {
	wxSizer* thickness_sizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* thickness_sub_sizer = newd wxBoxSizer(wxHORIZONTAL);
	thickness_sub_sizer->Add(20, 10);
	use_button = newd wxCheckBox(this, PALETTE_DOODAD_USE_THICKNESS, "Use custom thickness");
	use_button->SetToolTip("Enable custom thickness for this brush");
	thickness_sub_sizer->Add(use_button);
	thickness_sizer->Add(thickness_sub_sizer, 1, wxEXPAND);

	slider = newd wxSlider(this, PALETTE_DOODAD_SLIDER, 5, 1, 10, wxDefaultPosition);
	slider->SetToolTip("Adjust brush thickness");
	thickness_sizer->Add(slider, 1, wxEXPAND);

	SetSizerAndFit(thickness_sizer);
}

BrushThicknessPanel::~BrushThicknessPanel() {
	////
}

wxString BrushThicknessPanel::GetName() const {
	return "Brush Thickness";
}

void BrushThicknessPanel::OnScroll(wxScrollEvent& event) {
	static const int lookup_table[10] = { 1, 2, 3, 5, 8, 13, 23, 35, 50, 80 };
	use_button->SetValue(true);

	ASSERT(event.GetPosition() >= 1);
	ASSERT(event.GetPosition() <= 10);

	g_palettes.ActivatePalette(g_palettes.GetPalette());
	g_brush_manager.SetBrushThickness(true, lookup_table[event.GetPosition() - 1], 100);
}

void BrushThicknessPanel::OnClickCustomThickness(wxCommandEvent& event) {
	g_palettes.ActivatePalette(g_palettes.GetPalette());
	g_brush_manager.SetBrushThickness(event.IsChecked());
}

void BrushThicknessPanel::OnSwitchIn() {
	spdlog::info("BrushThicknessPanel::OnSwitchIn");
	static const int lookup_table[10] = { 1, 2, 3, 5, 8, 13, 23, 35, 50, 80 };
	g_brush_manager.SetBrushThickness(lookup_table[slider->GetValue() - 1], 100);
}
