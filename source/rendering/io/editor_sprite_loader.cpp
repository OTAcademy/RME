#include "app/definitions.h"
#include "rendering/io/editor_sprite_loader.h"
#include "rendering/core/graphics.h"
#include "rendering/core/editor_sprite.h"
#include "game/sprites.h"
#include "util/image_manager.h"
#include <memory>

// Helper logic moved to wx_utils.h
// Helper to wrap ImageManager returns for EditorSprite
#define getEditorSprite(pathSmall, pathLarge) std::make_unique<EditorSprite>( \
	std::make_unique<wxBitmap>(IMAGE_MANAGER.GetBitmap(pathSmall)),           \
	std::make_unique<wxBitmap>(IMAGE_MANAGER.GetBitmap(pathLarge))            \
)

#define getSingleEditorSprite(path) std::make_unique<EditorSprite>( \
	std::make_unique<wxBitmap>(IMAGE_MANAGER.GetBitmap(path)),      \
	nullptr                                                         \
)

bool EditorSpriteLoader::Load(GraphicManager* gm) {
	// Unused graphics MIGHT be loaded here, but it's a neglectable loss
	gm->insertSprite(EDITOR_SPRITE_SELECTION_MARKER, std::make_unique<EditorSprite>(std::make_unique<wxBitmap>(selection_marker_xpm16x16), std::make_unique<wxBitmap>(selection_marker_xpm32x32)));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_1x1, getEditorSprite(IMAGE_CIRCULAR_1_SMALL, IMAGE_CIRCULAR_1));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_3x3, getEditorSprite(IMAGE_CIRCULAR_2_SMALL, IMAGE_CIRCULAR_2));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_5x5, getEditorSprite(IMAGE_CIRCULAR_3_SMALL, IMAGE_CIRCULAR_3));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_7x7, getEditorSprite(IMAGE_CIRCULAR_4_SMALL, IMAGE_CIRCULAR_4));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_9x9, getEditorSprite(IMAGE_CIRCULAR_5_SMALL, IMAGE_CIRCULAR_5));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_15x15, getEditorSprite(IMAGE_CIRCULAR_6_SMALL, IMAGE_CIRCULAR_6));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_CD_19x19, getEditorSprite(IMAGE_CIRCULAR_7_SMALL, IMAGE_CIRCULAR_7));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_1x1, getEditorSprite(IMAGE_RECTANGULAR_1_SMALL, IMAGE_RECTANGULAR_1));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_3x3, getEditorSprite(IMAGE_RECTANGULAR_2_SMALL, IMAGE_RECTANGULAR_2));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_5x5, getEditorSprite(IMAGE_RECTANGULAR_3_SMALL, IMAGE_RECTANGULAR_3));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_7x7, getEditorSprite(IMAGE_RECTANGULAR_4_SMALL, IMAGE_RECTANGULAR_4));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_9x9, getEditorSprite(IMAGE_RECTANGULAR_5_SMALL, IMAGE_RECTANGULAR_5));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_15x15, getEditorSprite(IMAGE_RECTANGULAR_6_SMALL, IMAGE_RECTANGULAR_6));
	gm->insertSprite(EDITOR_SPRITE_BRUSH_SD_19x19, getEditorSprite(IMAGE_RECTANGULAR_7_SMALL, IMAGE_RECTANGULAR_7));

	gm->insertSprite(EDITOR_SPRITE_OPTIONAL_BORDER_TOOL, getEditorSprite(IMAGE_OPTIONAL_BORDER_SMALL, IMAGE_OPTIONAL_BORDER));
	gm->insertSprite(EDITOR_SPRITE_ERASER, getEditorSprite(IMAGE_ERASER_SMALL, IMAGE_ERASER));
	gm->insertSprite(EDITOR_SPRITE_PZ_TOOL, getEditorSprite(IMAGE_PROTECTION_ZONE_SMALL, IMAGE_PROTECTION_ZONE));
	gm->insertSprite(EDITOR_SPRITE_PVPZ_TOOL, getEditorSprite(IMAGE_PVP_ZONE_SMALL, IMAGE_PVP_ZONE));
	gm->insertSprite(EDITOR_SPRITE_NOLOG_TOOL, getEditorSprite(IMAGE_NO_LOGOUT_ZONE_SMALL, IMAGE_NO_LOGOUT_ZONE));
	gm->insertSprite(EDITOR_SPRITE_NOPVP_TOOL, getEditorSprite(IMAGE_NO_PVP_ZONE_SMALL, IMAGE_NO_PVP_ZONE));

	gm->insertSprite(EDITOR_SPRITE_DOOR_NORMAL, getEditorSprite(IMAGE_DOOR_NORMAL_SMALL, IMAGE_DOOR_NORMAL));
	gm->insertSprite(EDITOR_SPRITE_DOOR_LOCKED, getEditorSprite(IMAGE_DOOR_LOCKED_SMALL, IMAGE_DOOR_LOCKED));
	gm->insertSprite(EDITOR_SPRITE_DOOR_MAGIC, getEditorSprite(IMAGE_DOOR_MAGIC_SMALL, IMAGE_DOOR_MAGIC));
	gm->insertSprite(EDITOR_SPRITE_DOOR_QUEST, getEditorSprite(IMAGE_DOOR_QUEST_SMALL, IMAGE_DOOR_QUEST));
	gm->insertSprite(EDITOR_SPRITE_DOOR_NORMAL_ALT, getEditorSprite(IMAGE_DOOR_NORMAL_ALT_SMALL, IMAGE_DOOR_NORMAL_ALT));
	gm->insertSprite(EDITOR_SPRITE_DOOR_ARCHWAY, getEditorSprite(IMAGE_DOOR_ARCHWAY_SMALL, IMAGE_DOOR_ARCHWAY));
	gm->insertSprite(EDITOR_SPRITE_WINDOW_NORMAL, getEditorSprite(IMAGE_WINDOW_NORMAL_SMALL, IMAGE_WINDOW_NORMAL));
	gm->insertSprite(EDITOR_SPRITE_WINDOW_HATCH, getEditorSprite(IMAGE_WINDOW_HATCH_SMALL, IMAGE_WINDOW_HATCH));

	gm->insertSprite(EDITOR_SPRITE_SELECTION_GEM, getSingleEditorSprite(IMAGE_GEM_EDIT));
	gm->insertSprite(EDITOR_SPRITE_DRAWING_GEM, getSingleEditorSprite(IMAGE_GEM_MOVE));

	return true;
}
