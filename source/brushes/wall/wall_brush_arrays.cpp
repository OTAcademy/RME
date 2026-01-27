//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "brushes/wall/wall_brush.h"

//=============================================================================
// Border lookup tables

void WallBrush::init() {
	WallBrush::full_border_types[0] // 0
		= WALL_POLE;
	WallBrush::full_border_types[WALLTILE_NORTH] // 1
		= WALL_SOUTH_END;
	WallBrush::full_border_types[WALLTILE_WEST] // 10
		= WALL_EAST_END;
	WallBrush::full_border_types[WALLTILE_WEST | WALLTILE_NORTH] // 11
		= WALL_NORTHWEST_DIAGONAL;
	WallBrush::full_border_types[WALLTILE_EAST] // 100
		= WALL_WEST_END;
	WallBrush::full_border_types[WALLTILE_EAST | WALLTILE_NORTH] // 101
		= WALL_NORTHEAST_DIAGONAL;
	WallBrush::full_border_types[WALLTILE_EAST | WALLTILE_WEST] // 110
		= WALL_HORIZONTAL;
	WallBrush::full_border_types[WALLTILE_EAST | WALLTILE_WEST | WALLTILE_NORTH] // 111
		= WALL_SOUTH_T;
	WallBrush::full_border_types[WALLTILE_SOUTH] // 1000
		= WALL_NORTH_END;
	WallBrush::full_border_types[WALLTILE_SOUTH | WALLTILE_NORTH] // 1001
		= WALL_VERTICAL;
	WallBrush::full_border_types[WALLTILE_SOUTH | WALLTILE_WEST] // 1010
		= WALL_SOUTHWEST_DIAGONAL;
	WallBrush::full_border_types[WALLTILE_SOUTH | WALLTILE_WEST | WALLTILE_NORTH] // 1011
		= WALL_EAST_T;
	WallBrush::full_border_types[WALLTILE_SOUTH | WALLTILE_EAST] // 1100
		= WALL_SOUTHEAST_DIAGONAL;
	WallBrush::full_border_types[WALLTILE_SOUTH | WALLTILE_EAST | WALLTILE_NORTH] // 1101
		= WALL_WEST_T;
	WallBrush::full_border_types[WALLTILE_SOUTH | WALLTILE_EAST | WALLTILE_WEST] // 1110
		= WALL_NORTH_T;
	WallBrush::full_border_types[WALLTILE_SOUTH | WALLTILE_EAST | WALLTILE_WEST | WALLTILE_NORTH] // 1111
		= WALL_INTERSECTION;

	WallBrush::half_border_types[0] // 0
		= WALL_POLE;
	WallBrush::half_border_types[WALLTILE_NORTH] // 1
		= WALL_VERTICAL;
	WallBrush::half_border_types[WALLTILE_WEST] // 10
		= WALL_HORIZONTAL;
	WallBrush::half_border_types[WALLTILE_WEST | WALLTILE_NORTH] // 11
		= WALL_NORTHWEST_DIAGONAL;
	WallBrush::half_border_types[WALLTILE_EAST] // 100
		= WALL_POLE;
	WallBrush::half_border_types[WALLTILE_EAST | WALLTILE_NORTH] // 101
		= WALL_VERTICAL;
	WallBrush::half_border_types[WALLTILE_EAST | WALLTILE_WEST] // 110
		= WALL_HORIZONTAL;
	WallBrush::half_border_types[WALLTILE_EAST | WALLTILE_WEST | WALLTILE_NORTH] // 111
		= WALL_NORTHWEST_DIAGONAL;
	WallBrush::half_border_types[WALLTILE_SOUTH] // 1000
		= WALL_POLE;
	WallBrush::half_border_types[WALLTILE_SOUTH | WALLTILE_NORTH] // 1001
		= WALL_VERTICAL;
	WallBrush::half_border_types[WALLTILE_SOUTH | WALLTILE_WEST] // 1010
		= WALL_HORIZONTAL;
	WallBrush::half_border_types[WALLTILE_SOUTH | WALLTILE_WEST | WALLTILE_NORTH] // 1011
		= WALL_NORTHWEST_DIAGONAL;
	WallBrush::half_border_types[WALLTILE_SOUTH | WALLTILE_EAST] // 1100
		= WALL_POLE;
	WallBrush::half_border_types[WALLTILE_SOUTH | WALLTILE_EAST | WALLTILE_NORTH] // 1101
		= WALL_VERTICAL;
	WallBrush::half_border_types[WALLTILE_SOUTH | WALLTILE_EAST | WALLTILE_WEST] // 1110
		= WALL_HORIZONTAL;
	WallBrush::half_border_types[WALLTILE_SOUTH | WALLTILE_EAST | WALLTILE_WEST | WALLTILE_NORTH] // 1111
		= WALL_NORTHWEST_DIAGONAL;
}
