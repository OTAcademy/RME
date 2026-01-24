//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "rendering/utilities/light_drawer.h"
#include "rendering/utilities/light_calculator.h"
#include <glm/gtc/matrix_transform.hpp>
#include "tile.h"
#include "item.h"
#include "rendering/core/drawing_options.h"

LightDrawer::LightDrawer() {
}

LightDrawer::~LightDrawer() {
	unloadGLTexture();
	if (vao) {
		glDeleteVertexArrays(1, &vao);
	}
	if (vbo) {
		glDeleteBuffers(1, &vbo);
	}
}

#include "rendering/core/render_view.h"

void LightDrawer::draw(const RenderView& view, bool fog, const LightBuffer& light_buffer, const wxColor& global_color) {
	if (!texture.IsCreated()) {
		createGLTexture();
	}

	if (!shader) {
		initRenderResources();
	}

	int map_x = view.start_x;
	int map_y = view.start_y;
	int end_x = view.end_x;
	int end_y = view.end_y;

	int w = end_x - map_x;
	int h = end_y - map_y;

	if (w <= 0 || h <= 0) {
		return;
	}

	buffer.resize(static_cast<size_t>(w * h * PixelFormatRGBA));

	for (int x = 0; x < w; ++x) {
		for (int y = 0; y < h; ++y) {
			int mx = (map_x + x);
			int my = (map_y + y);
			int index = (y * w + x);
			int color_index = index * PixelFormatRGBA;

			buffer[color_index] = global_color.Red();
			buffer[color_index + 1] = global_color.Green();
			buffer[color_index + 2] = global_color.Blue();
			buffer[color_index + 3] = 140;

			for (const auto& light : light_buffer.lights) {
				float intensity = LightCalculator::calculateIntensity(mx, my, light);
				if (intensity == 0.f) {
					continue;
				}
				wxColor light_color = colorFromEightBit(light.color);
				uint8_t red = static_cast<uint8_t>(light_color.Red() * intensity);
				uint8_t green = static_cast<uint8_t>(light_color.Green() * intensity);
				uint8_t blue = static_cast<uint8_t>(light_color.Blue() * intensity);
				buffer[color_index] = std::max(buffer[color_index], red);
				buffer[color_index + 1] = std::max(buffer[color_index + 1], green);
				buffer[color_index + 2] = std::max(buffer[color_index + 2], blue);
			}
		}
	}

	// Draw Light Map
	shader->Use();

	// Calculate draw position and size based on RenderView
	// Note: TileSize in RenderView matches the zoomed size if we use projection matrix from it?
	// RenderView projection is orthogonal: 0..screensize
	// Coordinates in RenderView are usually unzoomed map coordinates * zoom?
	// Let's check view.projectionMatrix usage in SpriteBatch.
	// It seems RenderView sets up a projection where world units = screen pixels.
	// And tiles are drawn at (map_x * TileSize - scroll_x) * zoom ?
	// Actually MapDrawer uses glScalef(zoom, zoom, 1) in legacy OpenGL.
	// But SpriteBatch uses projectionMatrix.
	// RenderView::SetupGL() sets up glOrtho(0, screensize_x, screensize_y, 0, ...)
	// and viewMatrix likely contains the zoom and translation.

	// If we use view.projectionMatrix, we expect "world" coordinates.
	// The world coordinates for a tile at (map_x, map_y) are:
	// x = map_x * 32 - view.view_scroll_x
	// y = map_y * 32 - view.view_scroll_y
	// And then the view matrix applies zoom?

	// RenderView::projectionMatrix is typically just the Ortho projection.
	// RenderView::viewMatrix is where the camera transform logic resides, but we don't have it exposed simply as MVP usually?
	// Actually RenderView struct has 'viewMatrix' member!

	// Yes, check RenderView.h: glm::mat4 projectionMatrix; glm::mat4 viewMatrix;

	// So we should use uMVP = view.projectionMatrix * view.viewMatrix * model.

	int TileSize = 32; // CONSTANT base tile size
	// We calculate position in "world space" relative to the map origin (0,0) shifted by scroll?
	// Typically standard RME rendering (e.g. SpriteBatch) does:
	// batch->draw(texture, destRect, ...)
	// width/height of destRect are 32 * zoom? NO, SpriteBatch handles transform?
	// Actually SpriteBatch::begin(projection) takes just projection.
	// The coordinate system usually is screen coordinates if viewMatrix is Identity.

	// But wait, earlier we saw glOrtho(... width, height ...).
	// If MapDrawer iterates tiles and calls `glBlitSquare`, it sends coordinates:
	// draw_x = (map_x * TileSize - scroll_x)
	// And relies on GL_MODELVIEW scaling?
	// If SpriteBatch is used, it usually takes screen coordinates.
	// If RenderView has `zoom`, does it affect `projectionMatrix`?
	// Usually `projectionMatrix` is just 0..W, 0..H.

	// If `LightDrawer` manually calculates `draw_x` and `draw_y` using `map_x * TileSize - scroll_x`,
	// This is the "unzoomed" screen position if zoom was 1.0.
	// If zoom is != 1.0, we need to apply scaling.

	// `RenderView` struct has `zoom`.
	// The coordinates we want to draw to are:
	// X = (map_x * 32 - scroll_x) * zoom
	// Y = (map_y * 32 - scroll_y) * zoom
	// W = w * 32 * zoom
	// H = h * 32 * zoom

	// BUT, if we use `view.viewMatrix`, maybe it already has the zoom scaling?
	// Let's assume view.projectionMatrix is Screen Projection.
	// Let's assume we want to draw in Screen Space.

	// NOTE: RME scroll_x/y are usually in pixels (32px per tile).
	// RenderView uses: int draw_x, draw_y; view.getScreenPosition(map_x, map_y, map_z, draw_x, draw_y);
	// We should rely on that or replicate it.

	float zoom = view.zoom;

	// The quad covers tiles from map_x to map_x + w.
	// Screen X for map_x is:
	// (map_x * 32 - view.view_scroll_x) * zoom?
	// Or maybe scroll is already scaled? Standard RME uses unscaled scroll.

	// Replicating standard coordinate gen:
	int floor_adjustment = view.getFloorAdjustment();
	float draw_dest_x = (float)((map_x * TileSize - view.view_scroll_x - floor_adjustment));
	float draw_dest_y = (float)((map_y * TileSize - view.view_scroll_y - floor_adjustment));
	float draw_dest_w = (float)(w * TileSize);
	float draw_dest_h = (float)(h * TileSize);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(draw_dest_x, draw_dest_y, 0.0f));
	model = glm::scale(model, glm::vec3(draw_dest_w, draw_dest_h, 1.0f));

	// Use the projection matrix from the view (which should match window size) and view matrix (translation)
	// Actually RenderView::viewMatrix contains a translation (0.375, 0.375) for pixel alignment?
	// Let's include it.
	shader->SetMat4("uMVP", view.projectionMatrix * view.viewMatrix * model);
	shader->SetInt("uTexture", 0);

	texture.Bind(); // Active unit 0
	texture.SetFilter(GL_LINEAR, GL_LINEAR);
	texture.SetWrap(0x812F, 0x812F); // GL_CLAMP_TO_EDGE
	texture.Upload(w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

	if (!fog) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	}

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Restore default

	if (fog) {
		// Draw fog over the same area
		// Fog is just a colored quad.
		// We can use same shader, turn off texture?
		// Or simple multiply with a solid white texture?
		// Or use a uniform for color mixing.

		static GLuint whiteTex = 0;
		if (whiteTex == 0) {
			glGenTextures(1, &whiteTex);
			glBindTexture(GL_TEXTURE_2D, whiteTex);
			uint8_t white[] = { 255, 255, 255, 255 };
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
		}

		glm::vec4 fogColor(10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 80.0f / 255.0f);

		// Use shader with tint
		// I need to update shader to support tint then.
		// Or blend with constant color?
		// BatchRenderer::DrawQuad used a color shader.

		// I'll add "uColor" to shader.
		shader->SetVec4("uColor", fogColor);
		shader->SetInt("uUseTexture", 0); // 0 = color only

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glBindVertexArray(0);

		shader->SetInt("uUseTexture", 1); // Reset
		shader->SetVec4("uColor", glm::vec4(1.0f));
	}
}

void LightDrawer::initRenderResources() {
	const char* vs = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        out vec2 TexCoord;
        uniform mat4 uMVP;
        void main() {
            gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";
	const char* fs = R"(
        #version 330 core
        in vec2 TexCoord;
        out vec4 FragColor;
        uniform sampler2D uTexture;
        uniform vec4 uColor;
        uniform int uUseTexture;
        void main() {
            if (uUseTexture == 1) {
                FragColor = texture(uTexture, TexCoord) * uColor;
            } else {
                FragColor = uColor;
            }
        }
    )";

	shader = std::make_unique<ShaderProgram>();
	shader->Load(vs, fs);
	shader->Use();
	shader->SetVec4("uColor", glm::vec4(1.0f));
	shader->SetInt("uUseTexture", 1);

	float vertices[] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f
	};

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void LightDrawer::createGLTexture() {
	texture.Create();
}

void LightDrawer::unloadGLTexture() {
	texture.Release();
}
