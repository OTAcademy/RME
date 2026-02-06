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
	static bool LoadMetadata(GraphicManager* manager, const wxFileName& datafile, wxString& error, std::vector<std::string>& warnings);

private:
	static bool ReadSpriteGroup(GraphicManager* manager, FileReadHandle& file, GameSprite* sType, uint32_t group_index, std::vector<std::string>& warnings);
};

#endif
