#ifndef RME_EDITOR_PERSISTENCE_TILESET_EXPORTER_H
#define RME_EDITOR_PERSISTENCE_TILESET_EXPORTER_H

#include "util/common.h"
#include <wx/string.h>

class TilesetExporter {
public:
	static void exportTilesets(const FileName& directory, const std::string& filename);
};

#endif
