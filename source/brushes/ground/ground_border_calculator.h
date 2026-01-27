//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_GROUND_BORDER_CALCULATOR_H
#define RME_GROUND_BORDER_CALCULATOR_H

#include "app/main.h"

class BaseMap;
class Tile;

/**
 * @brief Handles the calculation of ground borders.
 */
class GroundBorderCalculator {
public:
	/**
	 * @brief Calculates and applies borders for a specific tile.
	 *
	 * @param map The map where the tile resides.
	 * @param tile The tile to calculate borders for.
	 */
	static void calculate(BaseMap* map, Tile* tile);
};

#endif // RME_GROUND_BORDER_CALCULATOR_H
