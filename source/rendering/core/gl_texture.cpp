#include "main.h"
#include "rendering/core/gl_texture.h"

GLTexture::GLTexture() :
	id(0) {
}

GLTexture::~GLTexture() {
	Release();
}

void GLTexture::Create() {
	if (id == 0) {
		glGenTextures(1, &id);
	}
}

void GLTexture::Release() {
	if (id != 0) {
		glDeleteTextures(1, &id);
		id = 0;
	}
}

void GLTexture::Bind() {
	if (id != 0) {
		glBindTexture(GL_TEXTURE_2D, id);
	}
}

void GLTexture::Upload(int width, int height, int format, int type, const void* data) {
	Bind();
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data);
}

void GLTexture::SetFilter(int min_filter, int mag_filter) {
	Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
}

void GLTexture::SetWrap(int wrap_s, int wrap_t) {
	Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
}
