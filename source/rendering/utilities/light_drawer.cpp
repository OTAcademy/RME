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
#include "rendering/core/batch_renderer.h"
#include "rendering/utilities/light_calculator.h"
#include "tile.h"
#include "item.h"
#include "rendering/core/drawing_options.h"

LightDrawer::LightDrawer() {
}

LightDrawer::~LightDrawer() {
	unloadGLTexture();
}

void LightDrawer::draw(int map_x, int map_y, int end_x, int end_y, int scroll_x, int scroll_y, bool fog, const LightBuffer& light_buffer, const wxColor& global_color) {
	if (!texture.IsCreated()) {
		texture.Create();
	}

	int w = end_x - map_x;
	int h = end_y - map_y;

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
			buffer[color_index + 3] = 140; // global_color.Alpha();

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

	const int draw_x = map_x * TileSize - scroll_x;
	const int draw_y = map_y * TileSize - scroll_y;
	int draw_width = w * TileSize;
	int draw_height = h * TileSize;

	texture.Bind();
	texture.SetFilter(GL_LINEAR, GL_LINEAR);
	texture.SetWrap(0x812F, 0x812F); // GL_CLAMP_TO_EDGE
	texture.Upload(w, h, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

	// Important: BatchRenderer might have other stuff pending. Flush it first.
	BatchRenderer::Flush();

	if (!fog) {
		glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	}

	BatchRenderer::DrawTextureQuad(
		glm::vec2(draw_x, draw_y),
		glm::vec2(draw_width, draw_height),
		glm::vec4(1.0f),
		texture.GetID()
	);

	// Flush IMMEDIATELY to apply the blend function to THIS quad
	BatchRenderer::Flush();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Restore default

	if (fog) {
		glm::vec4 fogColor(10.0f / 255.0f, 10.0f / 255.0f, 10.0f / 255.0f, 80.0f / 255.0f);
		BatchRenderer::DrawQuad(
			glm::vec2(draw_x, draw_y),
			glm::vec2(draw_width, draw_height),
			fogColor
		);
		// No need to explicit flush here, safe to batch
	}
}

void LightDrawer::createGLTexture() {
	texture.Create();
}

void LightDrawer::unloadGLTexture() {
	texture.Release();
}
