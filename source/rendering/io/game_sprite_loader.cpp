//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "rendering/io/game_sprite_loader.h"
#include "io/loaders/otfi_loader.h"
#include "io/loaders/dat_loader.h"
#include "io/loaders/spr_loader.h"

bool GameSpriteLoader::LoadOTFI(GraphicManager* manager, const wxFileName& filename, wxString& error, wxArrayString& warnings) {
	return OtfiLoader::Load(manager, filename, error, warnings);
}

bool GameSpriteLoader::LoadSpriteMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	return DatLoader::LoadMetadata(manager, datafile, error, warnings);
}

bool GameSpriteLoader::LoadSpriteData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings) {
	return SprLoader::LoadData(manager, datafile, error, warnings);
}

bool GameSpriteLoader::LoadSpriteDump(GraphicManager* manager, std::unique_ptr<uint8_t[]>& target, uint16_t& size, int sprite_id) {
	return SprLoader::LoadDump(manager, target, size, sprite_id);
}
