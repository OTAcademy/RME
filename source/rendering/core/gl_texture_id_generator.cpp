//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "rendering/core/gl_texture_id_generator.h"

GLuint GLTextureIDGenerator::GetFreeTextureID() {
	static GLuint id_counter = 0x10000000;
	return id_counter++; // This should (hopefully) never run out
}
