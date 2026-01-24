//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_CORE_GL_TEXTURE_ID_GENERATOR_H_
#define RME_RENDERING_CORE_GL_TEXTURE_ID_GENERATOR_H_

#include "app/main.h"

// Check if we are on Windows or Linux to include appropriate GL headers if main.h doesn't suffice or if we want to be explicit.
// Using GLuint which usually requires GL headers.
// main.h usually includes wx/wx.h or similar which might include GL headers via wxGLCanvas?
// Let's rely on main.h for now as graphics.h did.
// But actually we need GLuint type.
// On Windows it's usually in <GL/gl.h> or <GL/glut.h>.
// Let's use the same includes as graphics.cpp if possible or minimal set.

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

class GLTextureIDGenerator {
public:
	static GLuint GetFreeTextureID();
};

#endif
