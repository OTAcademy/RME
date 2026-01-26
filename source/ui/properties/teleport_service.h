//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_TELEPORT_SERVICE_H
#define RME_TELEPORT_SERVICE_H

#include "app/main.h"
#include "map/position.h"

class TeleportService {
public:
	static bool handlePositionPaste(int& x, int& y, int& z, uint32_t map_width, uint32_t map_height);
};

#endif
