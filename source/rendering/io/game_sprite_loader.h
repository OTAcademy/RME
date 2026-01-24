//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_IO_GAME_SPRITE_LOADER_H_
#define RME_RENDERING_IO_GAME_SPRITE_LOADER_H_

#include <wx/string.h>
#include <wx/arrstr.h> // For wxArrayString
#include <wx/filename.h> // For FileName if defined, or wxFileName

class GraphicManager;
class FileReadHandle;
class GameSprite;

class GameSpriteLoader {
public:
	static bool LoadOTFI(GraphicManager* manager, const wxFileName& filename, wxString& error, wxArrayString& warnings);
	static bool LoadSpriteMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings);
	static bool LoadSpriteData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings);
	static bool LoadSpriteDump(GraphicManager* manager, uint8_t*& target, uint16_t& size, int sprite_id);

private:
	static bool LoadSpriteMetadataFlags(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
};

#endif
