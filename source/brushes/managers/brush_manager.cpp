//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "brushes/managers/brush_manager.h"
#include "ui/gui.h"
#include "palette/palette_window.h"
#include "brushes/brush.h"
#include "brushes/spawn/spawn_brush.h"
#include "brushes/managers/doodad_preview_manager.h"
#include "palette/managers/palette_manager.h"
#include "ui/main_toolbar.h"

BrushManager g_brush_manager;

BrushManager::BrushManager() :
	house_brush(nullptr),
	house_exit_brush(nullptr),
	waypoint_brush(nullptr),
	optional_brush(nullptr),
	eraser(nullptr),
	normal_door_brush(nullptr),
	locked_door_brush(nullptr),
	magic_door_brush(nullptr),
	quest_door_brush(nullptr),
	hatch_door_brush(nullptr),
	window_door_brush(nullptr),
	normal_door_alt_brush(nullptr),
	archway_door_brush(nullptr),
	pz_brush(nullptr),
	rook_brush(nullptr),
	nolog_brush(nullptr),
	pvp_brush(nullptr),

	current_brush(nullptr),
	previous_brush(nullptr),
	brush_shape(BRUSHSHAPE_SQUARE),
	brush_size(0),
	brush_variation(0),
	creature_spawntime(0),
	draw_locked_doors(false),
	use_custom_thickness(false),
	custom_thickness_mod(0.0),
	light_intensity(1.0f),
	ambient_light_level(0.5f) {
}

BrushManager::~BrushManager() {
}

void BrushManager::SelectBrush() {
	if (g_palettes.palettes.empty()) {
		return;
	}

	SelectBrushInternal(g_palettes.palettes.front()->GetSelectedBrush());
	g_gui.RefreshView();
}

bool BrushManager::SelectBrush(const Brush* whatbrush, PaletteType primary) {
	if (g_palettes.palettes.empty()) {
		if (!g_palettes.CreatePalette()) {
			return false;
		}
	}

	if (!g_palettes.palettes.front()->OnSelectBrush(whatbrush, primary)) {
		return false;
	}

	SelectBrushInternal(const_cast<Brush*>(whatbrush));
	g_gui.root->GetAuiToolBar()->UpdateBrushButtons();
	return true;
}

void BrushManager::SelectBrushInternal(Brush* brush) {
	if (current_brush != brush && brush) {
		previous_brush = current_brush;
	}

	current_brush = brush;
	if (!current_brush) {
		return;
	}

	brush_variation = std::min(brush_variation, brush->getMaxVariation());
	g_doodad_preview.FillBuffer();
	if (brush->isDoodad()) {
		g_gui.secondary_map = g_doodad_preview.GetBufferMap();
	}

	g_gui.SetDrawingMode();
	g_gui.RefreshView();
}

void BrushManager::SelectPreviousBrush() {
	if (previous_brush) {
		SelectBrush(previous_brush);
	}
}

void BrushManager::Clear() {
	current_brush = nullptr;
	previous_brush = nullptr;

	house_brush = nullptr;
	house_exit_brush = nullptr;
	waypoint_brush = nullptr;
	optional_brush = nullptr;
	eraser = nullptr;
	spawn_brush = nullptr;
	normal_door_brush = nullptr;
	locked_door_brush = nullptr;
	magic_door_brush = nullptr;
	quest_door_brush = nullptr;
	hatch_door_brush = nullptr;
	normal_door_alt_brush = nullptr;
	archway_door_brush = nullptr;
	window_door_brush = nullptr;
	pz_brush = nullptr;
	rook_brush = nullptr;
	nolog_brush = nullptr;
	pvp_brush = nullptr;
}

BrushShape BrushManager::GetBrushShape() const {
	if (current_brush == spawn_brush) {
		return BRUSHSHAPE_SQUARE;
	}
	return brush_shape;
}

void BrushManager::SetBrushSizeInternal(int nz) {
	if (nz != brush_size && current_brush && current_brush->isDoodad() && !current_brush->oneSizeFitsAll()) {
		brush_size = nz;
		g_doodad_preview.FillBuffer();
		g_gui.secondary_map = g_doodad_preview.GetBufferMap();
	} else {
		brush_size = nz;
	}
}

void BrushManager::SetBrushSize(int nz) {
	SetBrushSizeInternal(nz);

	for (auto& palette : g_palettes.palettes) {
		palette->OnUpdateBrushSize(brush_shape, brush_size);
	}

	g_gui.root->GetAuiToolBar()->UpdateBrushSize(brush_shape, brush_size);
}

void BrushManager::SetBrushVariation(int nz) {
	if (nz != brush_variation && current_brush && current_brush->isDoodad()) {
		brush_variation = nz;
		g_doodad_preview.FillBuffer();
		g_gui.secondary_map = g_doodad_preview.GetBufferMap();
	}
}

void BrushManager::SetBrushShape(BrushShape bs) {
	if (bs != brush_shape && current_brush && current_brush->isDoodad() && !current_brush->oneSizeFitsAll()) {
		brush_shape = bs;
		g_doodad_preview.FillBuffer();
		g_gui.secondary_map = g_doodad_preview.GetBufferMap();
	}
	brush_shape = bs;

	for (auto& palette : g_palettes.palettes) {
		palette->OnUpdateBrushSize(brush_shape, brush_size);
	}

	g_gui.root->GetAuiToolBar()->UpdateBrushSize(brush_shape, brush_size);
}

void BrushManager::SetBrushThickness(bool on, int x, int y) {
	use_custom_thickness = on;

	if (x != -1 || y != -1) {
		custom_thickness_mod = float(std::max(x, 1)) / float(std::max(y, 1));
	}

	if (current_brush && current_brush->isDoodad()) {
		g_doodad_preview.FillBuffer();
	}

	g_gui.RefreshView();
}

void BrushManager::SetBrushThickness(int low, int ceil) {
	custom_thickness_mod = float(std::max(low, 1)) / float(std::max(ceil, 1));

	if (use_custom_thickness && current_brush && current_brush->isDoodad()) {
		g_doodad_preview.FillBuffer();
	}

	g_gui.RefreshView();
}

void BrushManager::DecreaseBrushSize(bool wrap) {
	switch (brush_size) {
		case 0:
			if (wrap) {
				SetBrushSize(11);
			}
			break;
		case 1:
			SetBrushSize(0);
			break;
		case 2:
		case 3:
			SetBrushSize(1);
			break;
		case 4:
		case 5:
			SetBrushSize(2);
			break;
		case 6:
		case 7:
			SetBrushSize(4);
			break;
		case 8:
		case 9:
		case 10:
			SetBrushSize(6);
			break;
		case 11:
		default:
			SetBrushSize(8);
			break;
	}
}

void BrushManager::IncreaseBrushSize(bool wrap) {
	switch (brush_size) {
		case 0:
			SetBrushSize(1);
			break;
		case 1:
			SetBrushSize(2);
			break;
		case 2:
		case 3:
			SetBrushSize(4);
			break;
		case 4:
		case 5:
			SetBrushSize(6);
			break;
		case 6:
		case 7:
			SetBrushSize(8);
			break;
		case 8:
		case 9:
		case 10:
			SetBrushSize(11);
			break;
		case 11:
		default:
			if (wrap) {
				SetBrushSize(0);
			}
			break;
	}
}

void BrushManager::SetDoorLocked(bool on) {
	draw_locked_doors = on;
	g_gui.RefreshView();
}

void BrushManager::FillDoodadPreviewBuffer() {
	g_doodad_preview.FillBuffer();
}
