//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/teleport_service.h"
#include "util/common.h"

bool TeleportService::handlePositionPaste(int& x, int& y, int& z, uint32_t map_width, uint32_t map_height) {
	Position position;
	if (posFromClipboard(position, map_width, map_height)) {
		x = position.x;
		y = position.y;
		z = position.z;
		return true;
	}
	return false;
}
