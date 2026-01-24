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
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include "rendering/core/sprite_light.h"
#include "rendering/core/light_buffer.h"
#include "rendering/core/gl_texture.h"
#include "rendering/core/shader_program.h"

struct DrawingOptions;
struct RenderView;
class TileLocation;
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
	GLuint vao = 0;
	GLuint vbo = 0;

	void initRenderResources();
};

#endif
