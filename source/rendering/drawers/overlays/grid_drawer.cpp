#include "rendering/drawers/overlays/grid_drawer.h"
#include "rendering/core/batch_renderer.h"

#include "rendering/core/render_view.h"
#include "rendering/core/drawing_options.h"
#include "definitions.h"
#include <wx/gdicmn.h>

void GridDrawer::DrawGrid(const RenderView& view, const DrawingOptions& options) {
	if (!options.show_grid) {
		return;
	}

	glm::vec4 color(1.0f, 1.0f, 1.0f, 0.5f); // 128/255 approx 0.5

	// Batch all horizontal lines
	for (int y = view.start_y; y < view.end_y; ++y) {
		float yPos = y * TileSize - view.view_scroll_y;
		float xStart = view.start_x * TileSize - view.view_scroll_x;
		float xEnd = view.end_x * TileSize - view.view_scroll_x;
		BatchRenderer::DrawLine(glm::vec2(xStart, yPos), glm::vec2(xEnd, yPos), color);
	}
	// Batch all vertical lines
	for (int x = view.start_x; x < view.end_x; ++x) {
		float xPos = x * TileSize - view.view_scroll_x;
		float yStart = view.start_y * TileSize - view.view_scroll_y;
		float yEnd = view.end_y * TileSize - view.view_scroll_y;
		BatchRenderer::DrawLine(glm::vec2(xPos, yStart), glm::vec2(xPos, yEnd), color);
	}
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

	// BatchRenderer doesn't support disabling GL_TEXTURE_2D state globally,
	// but DrawQuad uses white texture if no texture ID is provided or if specific non-textured method used.
	// DrawQuad uses whiteTextureID by default.

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
}

void GridDrawer::DrawNodeLoadingPlaceholder(int nd_map_x, int nd_map_y, const RenderView& view) {
	int cy = (nd_map_y)*TileSize - view.view_scroll_y - view.getFloorAdjustment();
	int cx = (nd_map_x)*TileSize - view.view_scroll_x - view.getFloorAdjustment();

	glm::vec4 color(1.0f, 0.0f, 1.0f, 0.5f); // 255, 0, 255, 128
	BatchRenderer::DrawQuad(
		glm::vec2(cx, cy),
		glm::vec2(TileSize * 4, TileSize * 4),
		color
	);
}

void GridDrawer::drawRect(int x, int y, int w, int h, const wxColor& color, int width) {
	// glLineWidth(width); // Width ignored for now, BatchRenderer lines are 1px
	glm::vec4 c(color.Red() / 255.0f, color.Green() / 255.0f, color.Blue() / 255.0f, color.Alpha() / 255.0f);

	// Top
	BatchRenderer::DrawLine(glm::vec2(x, y), glm::vec2(x + w, y), c);
	// Right
	BatchRenderer::DrawLine(glm::vec2(x + w, y), glm::vec2(x + w, y + h), c);
	// Bottom
	BatchRenderer::DrawLine(glm::vec2(x + w, y + h), glm::vec2(x, y + h), c);
	// Left
	BatchRenderer::DrawLine(glm::vec2(x, y + h), glm::vec2(x, y), c);
}

void GridDrawer::drawFilledRect(int x, int y, int w, int h, const wxColor& color) {
	glm::vec4 c(color.Red() / 255.0f, color.Green() / 255.0f, color.Blue() / 255.0f, color.Alpha() / 255.0f);
	BatchRenderer::DrawQuad(glm::vec2(x, y), glm::vec2(w, h), c);
}
