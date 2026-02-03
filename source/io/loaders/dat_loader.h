#ifndef RME_IO_LOADERS_DAT_LOADER_H_
#define RME_IO_LOADERS_DAT_LOADER_H_

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include "app/client_version.h"

class GraphicManager;
class FileReadHandle;
class GameSprite;

class DatLoader {
public:
	static bool LoadMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings);

private:
	static bool LoadMetadataFlags(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, wxString& error, wxArrayString& warnings);
	static uint8_t RemapFlag(uint8_t flag, DatFormat format);
	static bool ReadFlagData(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, uint8_t flag, wxArrayString& warnings);
};

#endif
