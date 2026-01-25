//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_BRUSH_MANAGER_H_
#define RME_BRUSH_MANAGER_H_

#include "app/main.h"
#include "brushes/brush_enums.h"

class Brush;
class HouseBrush;
class HouseExitBrush;
class WaypointBrush;
class OptionalBorderBrush;
class EraserBrush;
class SpawnBrush;
class DoorBrush;
class FlagBrush;

class BrushManager {
public:
	BrushManager();
	~BrushManager();

	// Selects the currently seleceted brush in the active palette
	void SelectBrush();
	// Updates the palette AND selects the brush, second parameter is first palette to look in
	// Returns true if the brush was found and selected
	bool SelectBrush(const Brush* brush, PaletteType pt = TILESET_UNKNOWN);
	// Selects the brush selected before the current brush
	void SelectPreviousBrush();
	// Only selects the brush, doesn't update the palette
	void SelectBrushInternal(Brush* brush);

	void Clear();
	void FillDoodadPreviewBuffer();

	// Get different brush parameters
	Brush* GetCurrentBrush() const {
		return current_brush;
	}
	BrushShape GetBrushShape() const;
	int GetBrushSize() const {
		return brush_size;
	}
	int GetBrushVariation() const {
		return brush_variation;
	}
	int GetSpawnTime() const {
		return creature_spawntime;
	}

	// Additional brush parameters
	void SetSpawnTime(int time) {
		creature_spawntime = time;
	}

	void SetLightIntensity(float v) {
		light_intensity = v;
	}
	float GetLightIntensity() const {
		return light_intensity;
	}

	void SetAmbientLightLevel(float v) {
		ambient_light_level = v;
	}
	float GetAmbientLightLevel() const {
		return ambient_light_level;
	}

	void SetBrushSize(int nz);
	void SetBrushSizeInternal(int nz);
	void SetBrushShape(BrushShape bs);
	void SetBrushVariation(int nz);
	void SetBrushThickness(int low, int ceil);
	void SetBrushThickness(bool on, int low = -1, int ceil = -1);

	// Helper functions for size
	void DecreaseBrushSize(bool wrap = false);
	void IncreaseBrushSize(bool wrap = false);

	// Door brush options
	void SetDoorLocked(bool on);
	bool HasDoorLocked() const {
		return draw_locked_doors;
	}

	float GetCustomThicknessMod() const {
		return custom_thickness_mod;
	}
	bool UseCustomThickness() const {
		return use_custom_thickness;
	}

public:
	// Brush references
	HouseBrush* house_brush;
	HouseExitBrush* house_exit_brush;
	WaypointBrush* waypoint_brush;
	OptionalBorderBrush* optional_brush;
	EraserBrush* eraser;
	SpawnBrush* spawn_brush;
	DoorBrush* normal_door_brush;
	DoorBrush* locked_door_brush;
	DoorBrush* magic_door_brush;
	DoorBrush* quest_door_brush;
	DoorBrush* hatch_door_brush;
	DoorBrush* normal_door_alt_brush;
	DoorBrush* archway_door_brush;
	DoorBrush* window_door_brush;
	FlagBrush* pz_brush;
	FlagBrush* rook_brush;
	FlagBrush* nolog_brush;
	FlagBrush* pvp_brush;

private:
	Brush* current_brush;
	Brush* previous_brush;
	BrushShape brush_shape;
	int brush_size;
	int brush_variation;
	int creature_spawntime;

	bool draw_locked_doors;
	bool use_custom_thickness;
	float custom_thickness_mod;
	float light_intensity;
	float ambient_light_level;
};

extern BrushManager g_brush_manager;

#endif
