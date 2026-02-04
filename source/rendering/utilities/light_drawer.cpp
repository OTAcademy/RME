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

#include "app/main.h"
#include "rendering/utilities/light_drawer.h"
#include "rendering/utilities/light_calculator.h"
#include <glm/gtc/matrix_transform.hpp>
#include <format>
#include "map/tile.h"
#include "game/item.h"
#include "rendering/core/drawing_options.h"
#include "rendering/core/render_view.h"

// GPULight struct moved to header

LightDrawer::LightDrawer() {
}

LightDrawer::~LightDrawer() {
	// Resources are RAII managed
}

void LightDrawer::InitFBO() {
	fbo = std::make_unique<GLFramebuffer>();
	fbo_texture = std::make_unique<GLTextureResource>(GL_TEXTURE_2D);

	// Initial dummy size
	ResizeFBO(32, 32);

	glNamedFramebufferTexture(fbo->GetID(), GL_COLOR_ATTACHMENT0, fbo_texture->GetID(), 0);

	GLenum status = glCheckNamedFramebufferStatus(fbo->GetID(), GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		spdlog::error("LightDrawer FBO Incomplete: {}", status);
	}
}

void LightDrawer::ResizeFBO(int width, int height) {
	if (width == buffer_width && height == buffer_height) {
		return;
	}

	buffer_width = width;
	buffer_height = height;

	glTextureStorage2D(fbo_texture->GetID(), 1, GL_RGBA8, width, height);

	// Set texture parameters
	glTextureParameteri(fbo_texture->GetID(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(fbo_texture->GetID(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(fbo_texture->GetID(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(fbo_texture->GetID(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void LightDrawer::draw(const RenderView& view, bool fog, const LightBuffer& light_buffer, const wxColor& global_color, float light_intensity, float ambient_light_level) {
	if (!shader) {
		initRenderResources();
	}

	if (!fbo) {
		InitFBO();
	}

	int map_x = view.start_x;
	int map_y = view.start_y;
	int end_x = view.end_x;
	int end_y = view.end_y;

	int w = end_x - map_x;
	int h = end_y - map_y;

	// Save previous state to allow composing into an external FBO (e.g. MapDrawer's scale_fbo)
	GLint old_draw_fbo, old_read_fbo;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_draw_fbo);
	glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &old_read_fbo);
	GLint old_viewport[4];
	glGetIntegerv(GL_VIEWPORT, old_viewport);

	if (w <= 0 || h <= 0) {
		return;
	}

	int TileSize = 32;
	int pixel_width = w * TileSize;
	int pixel_height = h * TileSize;

	// 1. Resize FBO if needed (ensure it covers the visible map area)
	// We check if current buffer is smaller than needed
	if (buffer_width < pixel_width || buffer_height < pixel_height) {
		// Re-create texture if we need to grow
		fbo_texture = std::make_unique<GLTextureResource>(GL_TEXTURE_2D);
		ResizeFBO(std::max(buffer_width, pixel_width), std::max(buffer_height, pixel_height));
		glNamedFramebufferTexture(fbo->GetID(), GL_COLOR_ATTACHMENT0, fbo_texture->GetID(), 0);
	}

	// 2. Prepare Lights
	// Filter and convert lights to GPU format
	gpu_lights_.clear();
	gpu_lights_.reserve(light_buffer.lights.size());

	float map_draw_start_x = (float)(map_x * TileSize - view.view_scroll_x);
	float map_draw_start_y = (float)(map_y * TileSize - view.view_scroll_y);

	// Offset logic:
	// The FBO represents the rectangle [map_x * 32, map_x * 32 + pixel_width] in map coordinates.
	// We want to render lights into this FBO.
	// Light Position in FBO = (LightMapX * 32 - FBO_StartX_In_Pixels, ...)
	// FBO Start X in pure Map Pixel Coords = map_x * 32.

	float fbo_origin_x = (float)(map_x * TileSize);
	float fbo_origin_y = (float)(map_y * TileSize);

	for (const auto& light : light_buffer.lights) {
		// Cull lights that are definitely out of FBO range
		// Radius approx light.intensity * TileSize
		int radius_px = light.intensity * TileSize + 16;
		int lx_px = light.map_x * TileSize + TileSize / 2;
		int ly_px = light.map_y * TileSize + TileSize / 2;

		// Check overlap with FBO rect
		if (lx_px + radius_px < fbo_origin_x || lx_px - radius_px > fbo_origin_x + pixel_width || ly_px + radius_px < fbo_origin_y || ly_px - radius_px > fbo_origin_y + pixel_height) {
			continue;
		}

		wxColor c = colorFromEightBit(light.color);

		// Position relative to FBO
		float rel_x = lx_px - fbo_origin_x;
		float rel_y = ly_px - fbo_origin_y;

		gpu_lights_.push_back({ .position = { rel_x, rel_y }, .intensity = static_cast<float>(light.intensity), .padding = 0.0f,
								// Pre-multiply intensity here if needed, or in shader
								.color = { (c.Red() / 255.0f) * light_intensity, (c.Green() / 255.0f) * light_intensity, (c.Blue() / 255.0f) * light_intensity, 1.0f } });
	}

	if (gpu_lights_.empty()) {
		// Just render ambient? We still need to clear the FBO/screen area or simpy fill it.
		// If no lights, the overlay should just be ambient color.
	} else {
		// Upload Lights
		size_t needed_size = gpu_lights_.size() * sizeof(GPULight);
		if (needed_size > light_ssbo_capacity_) {
			light_ssbo_capacity_ = std::max(needed_size, static_cast<size_t>(light_ssbo_capacity_ * 1.5));
			if (light_ssbo_capacity_ < 1024) {
				light_ssbo_capacity_ = 1024;
			}
			glNamedBufferData(light_ssbo->GetID(), light_ssbo_capacity_, nullptr, GL_DYNAMIC_DRAW);
		}
		glNamedBufferSubData(light_ssbo->GetID(), 0, needed_size, gpu_lights_.data());
	}

	// 3. Render to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, fbo->GetID());
	glViewport(0, 0, buffer_width, buffer_height); // Render to full buffer size, though we only care about top-left [pixel_width, pixel_height]

	// Clear to Ambient Color
	float ambient_r = (global_color.Red() / 255.0f) * ambient_light_level;
	float ambient_g = (global_color.Green() / 255.0f) * ambient_light_level;
	float ambient_b = (global_color.Blue() / 255.0f) * ambient_light_level;

	// If global_color is (0,0,0) (not set), use a default dark ambient
	if (global_color.Red() == 0 && global_color.Green() == 0 && global_color.Blue() == 0) {
		ambient_r = 0.5f * ambient_light_level;
		ambient_g = 0.5f * ambient_light_level;
		ambient_b = 0.5f * ambient_light_level;
	}

	glClearColor(ambient_r, ambient_g, ambient_b, 1.0f);
	// Actually, for "Max" blending, we want to start with Ambient.
	glClear(GL_COLOR_BUFFER_BIT);

	if (!gpu_lights_.empty()) {
		shader->Use();

		// Setup Projection for FBO: Ortho 0..buffer_width, buffer_height..0 (Y-down)
		// This matches screen coordinate system and avoids flips
		glm::mat4 fbo_projection = glm::ortho(0.0f, (float)buffer_width, (float)buffer_height, 0.0f);
		shader->SetMat4("uProjection", fbo_projection);
		shader->SetFloat("uTileSize", (float)TileSize);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, light_ssbo->GetID());
		glBindVertexArray(vao->GetID());

		// Enable MAX blending
		glEnable(GL_BLEND);
		glBlendEquation(GL_MAX);
		glBlendFunc(GL_ONE, GL_ONE); // Factors don't matter much for MAX, but usually 1,1 is safe

		glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, (GLsizei)gpu_lights_.size());

		glBlendEquation(GL_FUNC_ADD); // Restore default
		glBindVertexArray(0);
	}

	// Restore Previous FBO and Viewport
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, old_draw_fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, old_read_fbo);
	glViewport(old_viewport[0], old_viewport[1], old_viewport[2], old_viewport[3]);

	// 4. Composite FBO to Screen
	// Actually MapDrawer doesn't seem to set viewport every time, but `view.projectionMatrix` assumes 0..screensize.

	// Use PrimitiveRenderer or just a simple quad?
	// We can use a simple Blit shader or re-use `sprite_drawer->glBlitSquare` if it supports textures.
	// Or just do a quick manual draw using fixed pipeline or a simple shader.
	// Let's use `glBlitNamedFramebuffer`? No, we need blending (Multiply).
	// So we draw a textured quad.

	// We can reuse the `shader` but we need a "PASS THROUGH" mode or a separate shader.
	// Easier to just use `glEnable(GL_TEXTURE_2D)` and fixed function if compatible? No, we are in Core Profile likely.
	// Let's assume we need a simple texture shader.
	// BUT wait, `LightDrawer::draw` previously drew a quad with the computed light.
	// We can just use a simple "Texture Shader" here.

	// WARNING: We don't have a generic texture shader easily accessible here?
	// `floor_drawer` etc use `sprite_batch`.
	// We can use `sprite_batch` to draw the FBO texture!
	// `sprite_batch` usually takes Atlas Region.
	// Does `SpriteBatch` support raw texture ID?
	// `SpriteBatch` seems to assume Atlas.

	// Let's use the local `shader` with a "Mode" switch? Or just a second tiny shader.
	// Adding a mode switch to `shader` is easiest. "uMode": 0 = Light Render, 1 = Composite.

	shader->Use();
	shader->SetInt("uMode", 1); // Composite Mode

	// Bind FBO texture
	glBindTextureUnit(0, fbo_texture->GetID());
	shader->SetInt("uTexture", 0);

	// Quad Transform for Screen
	float draw_dest_x = map_draw_start_x;
	float draw_dest_y = map_draw_start_y;
	float draw_dest_w = (float)pixel_width;
	float draw_dest_h = (float)pixel_height;

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(draw_dest_x, draw_dest_y, 0.0f));
	model = glm::scale(model, glm::vec3(draw_dest_w, draw_dest_h, 1.0f));
	shader->SetMat4("uProjection", view.projectionMatrix * view.viewMatrix * model); // reusing uProjection as MVP

	// Calculate UVs based on used part of FBO
	// Since we use Y-down ortho, small Y is at Top of FBO.
	// OpenGL textures have Y=0 at Bottom.
	// So Map Top (Y=0) is at FBO Top (V=1 if using Y-down ortho with full height).
	// But we only use [0..pixel_height] of [0..buffer_height].
	// FBO Top is at Y=0. FBO Bottom is at Y=buffer_height.
	// If we use Y-down ortho: Y=0 -> V=1, Y=buffer_height -> V=0.
	// Map Top (Y=0) -> V=1. Map Bottom (Y=pixel_height) -> V = 1.0 - (pixel_height / buffer_height).

	float uv_w = (float)pixel_width / (float)buffer_width;
	float uv_h = (float)pixel_height / (float)buffer_height;

	// We pass UV range to shader. Shader should map aPos.y (0..1) to correct range.
	// If screen aPos.y=0 is Top, it should get texture Map Top.
	// With Y-down ortho into FBO, Map Top is at top of viewport, which is V=1 in GL.
	// So aPos.y=0 -> V=1, aPos.y=1 -> V = 1.0 - uv_h.
	shader->SetVec2("uUVMin", glm::vec2(0.0f, 1.0f));
	shader->SetVec2("uUVMax", glm::vec2(uv_w, 1.0f - uv_h));

	// Blending: Dst * Src
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	glBindVertexArray(vao->GetID());
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindVertexArray(0);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);

	shader->SetInt("uMode", 0); // Reset
}

void LightDrawer::initRenderResources() {
	// Modes: 0 = Light Generation (Instanced), 1 = Composite (Simple Texture)
	const char* vs = R"(
		#version 450 core
		layout (location = 0) in vec2 aPos; // 0..1 Quad
		
		uniform int uMode;
		uniform mat4 uProjection;
		uniform float uTileSize;
		uniform vec2 uUVMin;
		uniform vec2 uUVMax;

		struct Light {
			vec2 position; 
			float intensity;
			float padding;
			vec4 color;
		};
		layout(std430, binding = 0) buffer LightBlock {
			Light uLights[];
		};

		out vec2 TexCoord;
		out vec4 FragColor; // For Mode 0

		void main() {
			if (uMode == 0) {
				// LIGHT GENERATION
				Light l = uLights[gl_InstanceID];
				
				// Calculate quad size/pos in FBO space
				// Radius spans 0..1 in distance math
				// Quad size should cover the light radius
				// light.intensity is in 'tiles'. 
				// The falloff is 1.0 at center, 0.0 at radius = intensity * TileSize.
				float radiusPx = l.intensity * uTileSize;
				float size = radiusPx * 2.0;
				
				// Center position
				vec2 center = l.position;
				
				// Vertex Pos (0..1) -> Local Pos (-size/2 .. +size/2) -> World Pos
				vec2 localPos = (aPos - 0.5) * size;
				vec2 worldPos = center + localPos;
				
				gl_Position = uProjection * vec4(worldPos, 0.0, 1.0);
				
				// Pass data to fragment
				TexCoord = aPos - 0.5; // -0.5 to 0.5
				FragColor = l.color;
			} else {
				// COMPOSITE
				gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
				TexCoord = mix(uUVMin, uUVMax, aPos); 
			}
		}
	)";

	const char* fs = R"(
		#version 450 core
		in vec2 TexCoord;
		in vec4 FragColor; // From VS
		
		uniform int uMode;
		uniform sampler2D uTexture;

		out vec4 OutColor;

		void main() {
			if (uMode == 0) {
				// Light Falloff
				// TexCoord is -0.5 to 0.5
				float dist = length(TexCoord) * 2.0; // 0.0 to 1.0 (at edge of quad)
				if (dist > 1.0) discard;
				
				float falloff = 1.0 - dist;
				// Smooth it a bit?
				// falloff = offset - (distance * 0.2)? Legacy formula:
				// float intensity = (-distance + light.intensity) * 0.2f;
				// Here we just use linear falloff for simplicity or match formula
				// Visual approximation is fine for now.
				
				OutColor = FragColor * falloff;
			} else {
				// Texture fetch
				OutColor = texture(uTexture, TexCoord);
			}
		}
	)";

	shader = std::make_unique<ShaderProgram>();
	shader->Load(vs, fs);

	float vertices[] = {
		0.0f, 0.0f, // BL
		1.0f, 0.0f, // BR
		1.0f, 1.0f, // TR
		0.0f, 1.0f // TL
	};

	vao = std::make_unique<GLVertexArray>();
	vbo = std::make_unique<GLBuffer>();
	light_ssbo = std::make_unique<GLBuffer>();

	glNamedBufferData(vbo->GetID(), sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexArrayVertexBuffer(vao->GetID(), 0, vbo->GetID(), 0, 2 * sizeof(float));

	glEnableVertexArrayAttrib(vao->GetID(), 0);
	glVertexArrayAttribFormat(vao->GetID(), 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao->GetID(), 0, 0);

	glBindVertexArray(0);
}
