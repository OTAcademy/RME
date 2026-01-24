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

void LightDrawer::draw(const RenderView& view, bool fog, const LightBuffer& light_buffer, const wxColor& global_color, float light_intensity, float ambient_light_level) {
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

	shader->Use();

	// Set Global/Ambient Color
	glm::vec3 ambientColor(
		(global_color.Red() / 255.0f) * ambient_light_level,
		(global_color.Green() / 255.0f) * ambient_light_level,
		(global_color.Blue() / 255.0f) * ambient_light_level
	);
	shader->SetVec3("uAmbientColor", ambientColor);

	// Set Map Geometry Uniforms
	int TileSize = 32;
	int floor_adjustment = view.getFloorAdjustment();
	shader->SetFloat("uMapStartX", (float)(view.start_x * TileSize - view.view_scroll_x - floor_adjustment));
	shader->SetFloat("uMapStartY", (float)(view.start_y * TileSize - view.view_scroll_y - floor_adjustment));
	shader->SetFloat("uMapWidth", (float)(w * TileSize));
	shader->SetFloat("uMapHeight", (float)(h * TileSize));
	shader->SetFloat("uTileSize", (float)TileSize);

	// Populate Lights
	int lightCount = 0;
	char uniformName[64];

	// Sort lights by distance to center to ensure priority
	int centerX = (map_x + end_x) / 2;
	int centerY = (map_y + end_y) / 2;

	// Create a local copy for sorting
	// (LightBuffer is passed as const ref, so we need a copy)
	std::vector<LightBuffer::Light> sorted_lights = light_buffer.lights;

	std::sort(sorted_lights.begin(), sorted_lights.end(), [centerX, centerY](const LightBuffer::Light& a, const LightBuffer::Light& b) {
		int distA = (a.map_x - centerX) * (a.map_x - centerX) + (a.map_y - centerY) * (a.map_y - centerY);
		int distB = (b.map_x - centerX) * (b.map_x - centerX) + (b.map_y - centerY) * (b.map_y - centerY);
		return distA < distB;
	});

	for (const auto& light : sorted_lights) {
		if (lightCount >= 256) {
			break; // Max lights
		}

		// Check visibility - CPU Cull
		// Add safety margin to prevent popping at edges due to zoom/precision
		int radius = light.intensity + 5; // Increased margin
		if (light.map_x + radius < map_x || light.map_x - radius > end_x || light.map_y + radius < map_y || light.map_y - radius > end_y) {
			continue;
		}

		wxColor c = colorFromEightBit(light.color);
		glm::vec3 lightColor((c.Red() / 255.0f) * light_intensity, (c.Green() / 255.0f) * light_intensity, (c.Blue() / 255.0f) * light_intensity);

		// lightPos in screen coords (same as mapX/Y)
		// Note: light.map_x/y already includes perspective offset (from LightBuffer),
		// so we do NOT subtract floor_adjustment here.
		float lx = (float)(light.map_x * TileSize - view.view_scroll_x);
		float ly = (float)(light.map_y * TileSize - view.view_scroll_y);

		sprintf(uniformName, "uLights[%d].position", lightCount);
		shader->SetVec2(uniformName, glm::vec2(lx, ly));

		sprintf(uniformName, "uLights[%d].color", lightCount);
		shader->SetVec3(uniformName, lightColor);

		sprintf(uniformName, "uLights[%d].intensity", lightCount);
		shader->SetFloat(uniformName, (float)light.intensity);

		lightCount++;
	}
	shader->SetInt("uLightCount", lightCount);

	// Calculate Quad Transform
	float draw_dest_x = (float)((map_x * TileSize - view.view_scroll_x - floor_adjustment));
	float draw_dest_y = (float)((map_y * TileSize - view.view_scroll_y - floor_adjustment));
	float draw_dest_w = (float)(w * TileSize);
	float draw_dest_h = (float)(h * TileSize);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(draw_dest_x, draw_dest_y, 0.0f));
	model = glm::scale(model, glm::vec3(draw_dest_w, draw_dest_h, 1.0f));

	shader->SetMat4("uMVP", view.projectionMatrix * view.viewMatrix * model);

	// Blending: Multiply (Dst * Src).
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Restore

	if (fog) {
		// Fog logic integrated or dedicated pass needed.
		// For now, assuming standard pass covers most cases.
	}
}

void LightDrawer::initRenderResources() {
	// Shader for GPU-based lighting
	const char* vs = R"(
		#version 450 core
		layout (location = 0) in vec2 aPos;
		layout (location = 1) in vec2 aTexCoord;
		out vec2 TexCoord;
		out vec2 FragPos; // Screen space position
		uniform mat4 uMVP;
		uniform vec2 uScreenSize;
		uniform vec2 uViewScroll; // Un-zoomed scroll offset
		uniform float uZoom;
		uniform int uFloorAdjustment;

		void main() {
			gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
			TexCoord = aTexCoord;
		}
	)";

	const char* fs = R"(
		#version 450 core
		in vec2 TexCoord;
		out vec4 FragColor;

		struct Light {
			vec2 position; // Map coordinates * TileSize
			vec3 color;
			float intensity;
		};

		uniform vec3 uAmbientColor;
		uniform int uLightCount;
		uniform Light uLights[256]; // Max 256 visible lights

		// Uniforms to convert TexCoord/FragCoord to Map Coordinates
		uniform float uMapStartX;
		uniform float uMapStartY;
		uniform float uMapWidth;
		uniform float uMapHeight;
		uniform float uTileSize;

		void main() {
			// Calculate current pixel's position in map pixels (unzoomed)
			float mapX = uMapStartX + TexCoord.x * uMapWidth;
			float mapY = uMapStartY + TexCoord.y * uMapHeight;
			vec2 pixelPos = vec2(mapX, mapY);

			vec3 totalLight = uAmbientColor;

			for(int i = 0; i < uLightCount; i++) {
				float dist = distance(pixelPos, uLights[i].position);
				float radius = uLights[i].intensity * uTileSize;
				if (dist < radius) {
					float falloff = 1.0 - (dist / radius);
					vec3 lightColor = uLights[i].color * falloff;
					totalLight = max(totalLight, lightColor);
				}
			}

			// Output light color with appropriate alpha for blending
			FragColor = vec4(totalLight, 140.0/255.0);
		}
	)";

	shader = std::make_unique<ShaderProgram>();
	shader->Load(vs, fs);

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
}

void LightDrawer::unloadGLTexture() {
}
