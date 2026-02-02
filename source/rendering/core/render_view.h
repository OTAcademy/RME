#ifndef RME_RENDERING_RENDER_VIEW_H_
#define RME_RENDERING_RENDER_VIEW_H_

class MapCanvas;
struct DrawingOptions;

#include <glm/glm.hpp>
#include "map/position.h"

struct RenderView {
	float zoom;
	int tile_size;
	int floor;

	int start_x, start_y, start_z;
	int end_x, end_y, end_z, superend_z;
	int view_scroll_x, view_scroll_y;
	int screensize_x, screensize_y;
	int viewport_x, viewport_y;

	Position camera_pos;

	int mouse_map_x, mouse_map_y;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	void Setup(MapCanvas* canvas, const DrawingOptions& options);
	void SetupGL();
	void ReleaseGL();
	void Clear();

	int getFloorAdjustment() const;
	bool IsTileVisible(int map_x, int map_y, int map_z, int& out_x, int& out_y) const;
	void getScreenPosition(int map_x, int map_y, int map_z, int& out_x, int& out_y) const;
};

#endif
