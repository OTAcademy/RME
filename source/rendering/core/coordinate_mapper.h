//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_COORDINATE_MAPPER_H_
#define RME_RENDERING_COORDINATE_MAPPER_H_

class CoordinateMapper {
public:
	static void ScreenToMap(int screen_x, int screen_y, int view_start_x, int view_start_y, double zoom, int floor, double scale_factor, int* map_x, int* map_y);
};

#endif
