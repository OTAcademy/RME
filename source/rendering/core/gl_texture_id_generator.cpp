//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "rendering/core/gl_texture_id_generator.h"

#include "rendering/core/graphics.h" // For GL headers
#include "rendering/core/batch_renderer.h" // For GL headers via graphics/batch

GLuint GLTextureIDGenerator::GetFreeTextureID() {
	GLuint id;
	glGenTextures(1, &id);
	return id;
}
