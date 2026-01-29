#include "app/main.h"
#include "palette/panels/brush_size_panel.h"
#include "ui/gui.h"
#include "palette/managers/palette_manager.h"
#include "brushes/managers/brush_manager.h"
#include "palette/palette_window.h" // For PaletteWindow if needed for generic casting in GetParentPalette or similar?
// Actually PalettePanel::GetParentPalette returns PaletteWindow*, so we need the full definition of PaletteWindow likely,
// OR at least PaletteCommon needs to include it?
// palette_common.h forward declares PaletteWindow.
// palette_common.cpp includes palette_window.h via palette_common.h? No.
// palette_common.cpp doesn't seem to include palette_window.h directly but maybe indirectly?
// Ah, palette_common.cpp includes palette_window.h? NO.
// Wait, palette_common.cpp line 51 uses PaletteWindow.
// "const wxWindow* w... dynamic_cast<const PaletteWindow*>(w)"
// So it needs the definition.
// I will include "palette/palette_window.h" just in case or rely on "palette_common.h" if it pulls it in (it doesn't seems so).
// Actually let's include "palette/palette_window.h".
#include <spdlog/spdlog.h>

#include "palette/palette_window.h"
#include "game/sprites.h"

// ============================================================================
// Size Page

BEGIN_EVENT_TABLE(BrushSizePanel, wxPanel)
EVT_TOGGLEBUTTON(PALETTE_BRUSHSHAPE_SQUARE, BrushSizePanel::OnClickSquareBrush)
EVT_TOGGLEBUTTON(PALETTE_BRUSHSHAPE_CIRCLE, BrushSizePanel::OnClickCircleBrush)

EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_0, BrushSizePanel::OnClickBrushSize0)
EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_1, BrushSizePanel::OnClickBrushSize1)
EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_2, BrushSizePanel::OnClickBrushSize2)
EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_4, BrushSizePanel::OnClickBrushSize4)
EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_6, BrushSizePanel::OnClickBrushSize6)
EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_8, BrushSizePanel::OnClickBrushSize8)
EVT_TOGGLEBUTTON(PALETTE_TERRAIN_BRUSHSIZE_11, BrushSizePanel::OnClickBrushSize11)
END_EVENT_TABLE()

BrushSizePanel::BrushSizePanel(wxWindow* parent) :
	PalettePanel(parent, wxID_ANY) {
	////
}

void BrushSizePanel::InvalidateContents() {
	if (loaded) {
		DestroyChildren();
		SetSizer(nullptr);

		brushshapeSquareButton = brushshapeCircleButton = brushsize0Button = brushsize1Button = brushsize2Button = brushsize4Button = brushsize6Button = brushsize8Button = brushsize11Button = nullptr;

		loaded = false;
	}
}

void BrushSizePanel::LoadCurrentContents() {
	LoadAllContents();
}

void BrushSizePanel::LoadAllContents() {
	if (loaded) {
		return;
	}

	wxSizer* size_sizer = newd wxBoxSizer(wxVERTICAL);
	;
	wxSizer* sub_sizer = newd wxBoxSizer(wxHORIZONTAL);
	RenderSize render_size;

	if (large_icons) {
		// 32x32
		render_size = RENDER_SIZE_32x32;
	} else {
		// 16x16
		render_size = RENDER_SIZE_16x16;
	}

	sub_sizer->Add(brushshapeSquareButton = newd DCButton(this, PALETTE_BRUSHSHAPE_SQUARE, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_9x9));
	brushshapeSquareButton->SetToolTip("Square brush");

	sub_sizer->Add(brushshapeCircleButton = newd DCButton(this, PALETTE_BRUSHSHAPE_CIRCLE, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_CD_9x9));
	brushshapeCircleButton->SetToolTip("Circle brush");
	brushshapeSquareButton->SetValue(true);

	if (large_icons) {
		sub_sizer->AddSpacer(36);
	} else {
		sub_sizer->AddSpacer(18);
	}

	sub_sizer->Add(brushsize0Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_0, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_CD_1x1));
	brushsize0Button->SetToolTip("Brush size 1");
	brushsize0Button->SetValue(true);

	sub_sizer->Add(brushsize1Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_1, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_3x3));
	brushsize1Button->SetToolTip("Brush size 2");

	if (large_icons) {
		size_sizer->Add(sub_sizer);
		sub_sizer = newd wxBoxSizer(wxHORIZONTAL);
	}

	sub_sizer->Add(brushsize2Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_2, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_5x5));
	brushsize2Button->SetToolTip("Brush size 3");

	sub_sizer->Add(brushsize4Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_4, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_7x7));
	brushsize4Button->SetToolTip("Brush size 5");

	sub_sizer->Add(brushsize6Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_6, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_9x9));
	brushsize6Button->SetToolTip("Brush size 7");

	sub_sizer->Add(brushsize8Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_8, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_15x15));
	brushsize8Button->SetToolTip("Brush size 9");

	sub_sizer->Add(brushsize11Button = newd DCButton(this, PALETTE_TERRAIN_BRUSHSIZE_11, wxDefaultPosition, DC_BTN_TOGGLE, render_size, EDITOR_SPRITE_BRUSH_SD_19x19));
	brushsize11Button->SetToolTip("Brush size 12");

	size_sizer->Add(sub_sizer);
	SetSizerAndFit(size_sizer);

	loaded = true;
}

wxString BrushSizePanel::GetName() const {
	return "Brush Size";
}

void BrushSizePanel::SetToolbarIconSize(bool d) {
	InvalidateContents();
	large_icons = d;
}

void BrushSizePanel::OnSwitchIn() {
	spdlog::info("BrushSizePanel::OnSwitchIn");
	LoadCurrentContents();
}

void BrushSizePanel::OnUpdateBrushSize(BrushShape shape, int size) {
	spdlog::info("BrushSizePanel::OnUpdateBrushSize: loaded={}", loaded);
	if (!loaded) {
		return;
	}

	if (shape == BRUSHSHAPE_SQUARE) {
		brushshapeCircleButton->SetValue(false);
		brushshapeSquareButton->SetValue(true);

		brushsize0Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_1x1);
		brushsize1Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_3x3);
		brushsize2Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_5x5);
		brushsize4Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_7x7);
		brushsize6Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_9x9);
		brushsize8Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_15x15);
		brushsize11Button->SetSprite(EDITOR_SPRITE_BRUSH_SD_19x19);
	} else {
		brushshapeSquareButton->SetValue(false);
		brushshapeCircleButton->SetValue(true);

		brushsize0Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_1x1);
		brushsize1Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_3x3);
		brushsize2Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_5x5);
		brushsize4Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_7x7);
		brushsize6Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_9x9);
		brushsize8Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_15x15);
		brushsize11Button->SetSprite(EDITOR_SPRITE_BRUSH_CD_19x19);
	}

	if (brushsize0Button) {
		brushsize0Button->SetValue(false);
	}
	if (brushsize1Button) {
		brushsize1Button->SetValue(false);
	}
	if (brushsize2Button) {
		brushsize2Button->SetValue(false);
	}
	if (brushsize4Button) {
		brushsize4Button->SetValue(false);
	}
	if (brushsize6Button) {
		brushsize6Button->SetValue(false);
	}
	if (brushsize8Button) {
		brushsize8Button->SetValue(false);
	}
	if (brushsize11Button) {
		brushsize11Button->SetValue(false);
	}

	switch (size) {
		case 0:
			brushsize0Button->SetValue(true);
			break;
		case 1:
			brushsize1Button->SetValue(true);
			break;
		case 2:
			brushsize2Button->SetValue(true);
			break;
		case 4:
			brushsize4Button->SetValue(true);
			break;
		case 6:
			brushsize6Button->SetValue(true);
			break;
		case 8:
			brushsize8Button->SetValue(true);
			break;
		case 11:
			brushsize11Button->SetValue(true);
			break;
		default:
			brushsize0Button->SetValue(true);
			break;
	}
}

void BrushSizePanel::OnClickCircleBrush(wxCommandEvent& event) {
	g_palettes.ActivatePalette(g_palettes.GetPalette());
	g_brush_manager.SetBrushShape(BRUSHSHAPE_CIRCLE);
}

void BrushSizePanel::OnClickSquareBrush(wxCommandEvent& event) {
	g_palettes.ActivatePalette(g_palettes.GetPalette());
	g_brush_manager.SetBrushShape(BRUSHSHAPE_SQUARE);
}

void BrushSizePanel::OnClickBrushSize(int which) {
	g_palettes.ActivatePalette(g_palettes.GetPalette());
	g_brush_manager.SetBrushSize(which);
}
