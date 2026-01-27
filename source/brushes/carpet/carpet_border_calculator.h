//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_CARPET_BORDER_CALCULATOR_H
#define RME_CARPET_BORDER_CALCULATOR_H

#include <cstdint>

class BaseMap;
class Tile;

class CarpetBorderCalculator {
public:
	static void calculate(BaseMap* map, Tile* tile);
};

#endif
