//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_TILE_DESCRIBER_H_
#define RME_RENDERING_TILE_DESCRIBER_H_

#include <wx/string.h>

class Tile;
class Map;

class TileDescriber {
public:
	static wxString GetDescription(Tile* tile, bool showSpawns, bool showCreatures);
};

#endif
