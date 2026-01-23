#include "definitions.h"
#include "rendering/io/editor_sprite_loader.h"
#include "rendering/core/graphics.h"
#include "rendering/core/editor_sprite.h"
#include "sprites.h"
#include "../../../brushes/door_normal.xpm"
#include "../../../brushes/door_normal_small.xpm"
#include "../../../brushes/door_locked.xpm"
#include "../../../brushes/door_locked_small.xpm"
#include "../../../brushes/door_magic.xpm"
#include "../../../brushes/door_magic_small.xpm"
#include "../../../brushes/door_quest.xpm"
#include "../../../brushes/door_quest_small.xpm"
#include "../../../brushes/door_normal_alt.xpm"
#include "../../../brushes/door_normal_alt_small.xpm"
#include "../../../brushes/door_archway.xpm"
#include "../../../brushes/door_archway_small.xpm"
#include "main.h"

// Needs to be defined or included to access png files
#include "pngfiles.h"

#include <wx/mstream.h>

// Helper logic duplicated or moved from graphics.cpp
#define loadPNGFile(name) _wxGetBitmapFromMemory(name, sizeof(name))
inline wxBitmap* _wxGetBitmapFromMemory(const unsigned char* data, int length) {
	wxMemoryInputStream is(data, length);
	wxImage img(is, "image/png");
	if (!img.IsOk()) {
		return nullptr;
	}
	return newd wxBitmap(img, -1);
}

bool EditorSpriteLoader::Load(GraphicManager* gm) {
	// Unused graphics MIGHT be loaded here, but it's a neglectable loss
	gm->insertSprite(EDITOR_SPRITE_SELECTION_MARKER, newd EditorSprite(newd wxBitmap(selection_marker_xpm16x16), newd wxBitmap(selection_marker_xpm32x32)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_1x1, newd EditorSprite(loadPNGFile(circular_1_small_png), loadPNGFile(circular_1_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_3x3, newd EditorSprite(loadPNGFile(circular_2_small_png), loadPNGFile(circular_2_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_5x5, newd EditorSprite(loadPNGFile(circular_3_small_png), loadPNGFile(circular_3_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_7x7, newd EditorSprite(loadPNGFile(circular_4_small_png), loadPNGFile(circular_4_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_9x9, newd EditorSprite(loadPNGFile(circular_5_small_png), loadPNGFile(circular_5_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_15x15, newd EditorSprite(loadPNGFile(circular_6_small_png), loadPNGFile(circular_6_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_19x19, newd EditorSprite(loadPNGFile(circular_7_small_png), loadPNGFile(circular_7_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_1x1, newd EditorSprite(loadPNGFile(rectangular_1_small_png), loadPNGFile(rectangular_1_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_3x3, newd EditorSprite(loadPNGFile(rectangular_2_small_png), loadPNGFile(rectangular_2_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_5x5, newd EditorSprite(loadPNGFile(rectangular_3_small_png), loadPNGFile(rectangular_3_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_7x7, newd EditorSprite(loadPNGFile(rectangular_4_small_png), loadPNGFile(rectangular_4_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_9x9, newd EditorSprite(loadPNGFile(rectangular_5_small_png), loadPNGFile(rectangular_5_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_15x15, newd EditorSprite(loadPNGFile(rectangular_6_small_png), loadPNGFile(rectangular_6_png)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_19x19, newd EditorSprite(loadPNGFile(rectangular_7_small_png), loadPNGFile(rectangular_7_png)));

	gm->insertSprite(EDITOR_SPRITE_OPTIONAL_BORDER_TOOL, newd EditorSprite(loadPNGFile(optional_border_small_png), loadPNGFile(optional_border_png)));
	gm->insertSprite(EDITOR_SPRITE_ERASER, newd EditorSprite(loadPNGFile(eraser_small_png), loadPNGFile(eraser_png)));
	gm->insertSprite(EDITOR_SPRITE_PZ_TOOL, newd EditorSprite(loadPNGFile(protection_zone_small_png), loadPNGFile(protection_zone_png)));
	gm->insertSprite(EDITOR_SPRITE_PVPZ_TOOL, newd EditorSprite(loadPNGFile(pvp_zone_small_png), loadPNGFile(pvp_zone_png)));
	gm->insertSprite(EDITOR_SPRITE_NOLOG_TOOL, newd EditorSprite(loadPNGFile(no_logout_small_png), loadPNGFile(no_logout_png)));
	gm->insertSprite(EDITOR_SPRITE_NOPVP_TOOL, newd EditorSprite(loadPNGFile(no_pvp_small_png), loadPNGFile(no_pvp_png)));

	gm->insertSprite(EDITOR_SPRITE_DOOR_NORMAL, newd EditorSprite(newd wxBitmap(door_normal_small_xpm), newd wxBitmap(door_normal_xpm)));
	gm->insertSprite(EDITOR_SPRITE_DOOR_LOCKED, newd EditorSprite(newd wxBitmap(door_locked_small_xpm), newd wxBitmap(door_locked_xpm)));
	gm->insertSprite(EDITOR_SPRITE_DOOR_MAGIC, newd EditorSprite(newd wxBitmap(door_magic_small_xpm), newd wxBitmap(door_magic_xpm)));
	gm->insertSprite(EDITOR_SPRITE_DOOR_QUEST, newd EditorSprite(newd wxBitmap(door_quest_small_xpm), newd wxBitmap(door_quest_xpm)));
	gm->insertSprite(EDITOR_SPRITE_DOOR_NORMAL_ALT, newd EditorSprite(newd wxBitmap(door_normal_alt_small_xpm), newd wxBitmap(door_normal_alt_xpm)));
	gm->insertSprite(EDITOR_SPRITE_DOOR_ARCHWAY, newd EditorSprite(newd wxBitmap(door_archway_small_xpm), newd wxBitmap(door_archway_xpm)));
	gm->insertSprite(EDITOR_SPRITE_WINDOW_NORMAL, newd EditorSprite(loadPNGFile(window_normal_small_png), loadPNGFile(window_normal_png)));
	gm->insertSprite(EDITOR_SPRITE_WINDOW_HATCH, newd EditorSprite(loadPNGFile(window_hatch_small_png), loadPNGFile(window_hatch_png)));

	gm->insertSprite(EDITOR_SPRITE_SELECTION_GEM, newd EditorSprite(loadPNGFile(gem_edit_png), nullptr));
	gm->insertSprite(EDITOR_SPRITE_DRAWING_GEM, newd EditorSprite(loadPNGFile(gem_move_png), nullptr));

	return true;
}
