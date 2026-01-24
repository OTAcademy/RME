#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "rendering/drawers/cursors/brush_cursor_drawer.h"
#include "brush.h"
#include "definitions.h" // For PI

#include "rendering/core/sprite_batch.h"
#include "rendering/core/primitive_renderer.h"
#include "rendering/core/graphics.h"
#include "gui.h"

void BrushCursorDrawer::draw(SpriteBatch& sprite_batch, PrimitiveRenderer& primitive_renderer, int x, int y, Brush* brush, uint8_t r, uint8_t g, uint8_t b) {
	x += (TileSize / 2);
	y += (TileSize / 2);

	// 7----0----1
	// |         |
	// 6--5  3--2
	//     \/
	//     4
	static int vertexes[9][2] = {
		{ -15, -20 }, // 0
		{ 15, -20 }, // 1
		{ 15, -5 }, // 2
		{ 5, -5 }, // 3
		{ 0, 0 }, // 4
		{ -5, -5 }, // 5
		{ -15, -5 }, // 6
		{ -15, -20 }, // 7
		{ -15, -20 }, // 0
	};

	// circle
	glm::vec4 circleColor(0.0f, 0.0f, 0.0f, 0x50 / 255.0f);
	// Center: x,y.
	// Radius: TileSize/2.
	// Draw fan as triangles.
	float radius = TileSize / 2.0f;
	int segments = 30;
	glm::vec2 center(x, y);

	for (int i = 0; i < segments; i++) {
		float angle1 = i * 2.0f * PI / segments;
		float angle2 = (i + 1) * 2.0f * PI / segments;

		primitive_renderer.drawTriangle(
			center,
			glm::vec2(cos(angle1) * radius + x, sin(angle1) * radius + y),
			glm::vec2(cos(angle2) * radius + x, sin(angle2) * radius + y),
			circleColor
		);
	}

	// background
	glm::vec4 bgColor(r / 255.0f, g / 255.0f, b / 255.0f, 0xB4 / 255.0f);

	// Decomposed:
	// Box: (-15, -20) to (15, -5). width=30, height=15.
	// Top Left of box relative to x,y: (-15, -20).
	// But coordinates seem to be relative offset.
	if (g_gui.gfx.ensureAtlasManager()) {
		const AtlasManager& atlas = *g_gui.gfx.getAtlasManager();
		sprite_batch.drawRect((float)(x - 15), (float)(y - 20), (float)30, (float)15, bgColor, atlas);
	}

	// Tip Triangle: (-5, -5), (0,0), (5, -5) relative to x,y
	// Tip Triangle: (-5, -5), (0,0), (5, -5) relative to x,y
	primitive_renderer.drawTriangle(
		glm::vec2(x - 5, y - 5),
		glm::vec2(x, y),
		glm::vec2(x + 5, y - 5),
		bgColor
	);

	// borders
	glm::vec4 borderColor(0.0f, 0.0f, 0.0f, 0xB4 / 255.0f);

	for (int i = 0; i < 8; ++i) {
		primitive_renderer.drawLine(
			glm::vec2(vertexes[i][0] + x, vertexes[i][1] + y),
			glm::vec2(vertexes[i + 1][0] + x, vertexes[i + 1][1] + y),
			borderColor
		);
	}
}
