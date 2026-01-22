#include "main.h"
#include "rendering/grid_drawer.h"

#include "rendering/render_view.h"
#include "rendering/drawing_options.h"
#include "definitions.h"
#include <wx/gdicmn.h>

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

void GridDrawer::DrawGrid(const RenderView& view, const DrawingOptions& options) {
	if (!options.show_grid) {
		return;
	}

	glColor4ub(255, 255, 255, 128);
	glBegin(GL_LINES);
	// Batch all horizontal lines
	for (int y = view.start_y; y < view.end_y; ++y) {
		glVertex2f(view.start_x * TileSize - view.view_scroll_x, y * TileSize - view.view_scroll_y);
		glVertex2f(view.end_x * TileSize - view.view_scroll_x, y * TileSize - view.view_scroll_y);
	}
	// Batch all vertical lines
	for (int x = view.start_x; x < view.end_x; ++x) {
		glVertex2f(x * TileSize - view.view_scroll_x, view.start_y * TileSize - view.view_scroll_y);
		glVertex2f(x * TileSize - view.view_scroll_x, view.end_y * TileSize - view.view_scroll_y);
	}
	glEnd();
}

void GridDrawer::DrawIngameBox(const RenderView& view, const DrawingOptions& options) {
	if (!options.show_ingame_box) {
		return;
	}

	int center_x = view.start_x + int(view.screensize_x * view.zoom / 64);
	int center_y = view.start_y + int(view.screensize_y * view.zoom / 64);

	int offset_y = 2;
	int box_start_map_x = center_x;
	int box_start_map_y = center_y + offset_y;
	int box_end_map_x = center_x + ClientMapWidth;
	int box_end_map_y = center_y + ClientMapHeight + offset_y;

	int box_start_x = box_start_map_x * TileSize - view.view_scroll_x;
	int box_start_y = box_start_map_y * TileSize - view.view_scroll_y;
	int box_end_x = box_end_map_x * TileSize - view.view_scroll_x;
	int box_end_y = box_end_map_y * TileSize - view.view_scroll_y;

	static wxColor side_color(0, 0, 0, 200);

	glDisable(GL_TEXTURE_2D);

	// left side
	if (box_start_map_x >= view.start_x) {
		drawFilledRect(0, 0, box_start_x, view.screensize_y * view.zoom, side_color);
	}

	// right side
	if (box_end_map_x < view.end_x) {
		drawFilledRect(box_end_x, 0, view.screensize_x * view.zoom, view.screensize_y * view.zoom, side_color);
	}

	// top side
	if (box_start_map_y >= view.start_y) {
		drawFilledRect(box_start_x, 0, box_end_x - box_start_x, box_start_y, side_color);
	}

	// bottom side
	if (box_end_map_y < view.end_y) {
		drawFilledRect(box_start_x, box_end_y, box_end_x - box_start_x, view.screensize_y * view.zoom, side_color);
	}

	// hidden tiles
	drawRect(box_start_x, box_start_y, box_end_x - box_start_x, box_end_y - box_start_y, *wxRED);

	// visible tiles
	box_start_x += TileSize;
	box_start_y += TileSize;
	box_end_x -= 1 * TileSize;
	box_end_y -= 1 * TileSize;
	drawRect(box_start_x, box_start_y, box_end_x - box_start_x, box_end_y - box_start_y, *wxGREEN);

	// player position
	box_start_x += (ClientMapWidth - 3) / 2 * TileSize;
	box_start_y += (ClientMapHeight - 3) / 2 * TileSize;
	box_end_x = box_start_x + TileSize;
	box_end_y = box_start_y + TileSize;
	drawRect(box_start_x, box_start_y, box_end_x - box_start_x, box_end_y - box_start_y, *wxGREEN);

	glEnable(GL_TEXTURE_2D);
}

void GridDrawer::drawRect(int x, int y, int w, int h, const wxColor& color, int width) {
	glLineWidth(width);
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
	glBegin(GL_LINE_STRIP);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glVertex2f(x, y);
	glEnd();
}

void GridDrawer::drawFilledRect(int x, int y, int w, int h, const wxColor& color) {
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x + w, y);
	glVertex2f(x + w, y + h);
	glVertex2f(x, y + h);
	glEnd();
}
