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

#ifndef RME_LIGHDRAWER_H
#define RME_LIGHDRAWER_H

#include <cstdint>
#include <vector>
#include <memory>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <glm/glm.hpp>
#include "rendering/core/sprite_light.h"
#include "rendering/core/light_buffer.h"
#include "rendering/core/gl_texture.h"
#include "rendering/core/shader_program.h"
#include "rendering/core/gl_resources.h"

struct DrawingOptions;
struct RenderView;
class TileLocation;

struct GPULight {
	glm::vec2 position; // 8 bytes (offset 0)
	float intensity; // 4 bytes (offset 8)
	float padding; // 4 bytes (offset 12) -> Aligns color to 16 bytes
	glm::vec4 color; // 16 bytes (offset 16) -> Total 32 bytes
};

class LightDrawer {
public:
	LightDrawer();
	~LightDrawer();
	void draw(const RenderView& view, bool fog, const LightBuffer& light_buffer, const wxColor& global_color, float light_intensity = 1.0f, float ambient_light_level = 0.5f);

	void createGLTexture();
	void unloadGLTexture();

private:
	// wxColor global_color; // Removed state

	// Open GL Texture used for lightmap
	// It is owned by this class and should be released when context is destroyed

	std::unique_ptr<ShaderProgram> shader;
	std::unique_ptr<GLVertexArray> vao;
	std::unique_ptr<GLBuffer> vbo;
	std::unique_ptr<GLBuffer> light_ssbo;

	std::vector<GPULight> gpu_lights_;

	void initRenderResources();
};

#endif
