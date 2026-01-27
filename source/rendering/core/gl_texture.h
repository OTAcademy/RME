#ifndef RME_RENDERING_CORE_GL_TEXTURE_H_
#define RME_RENDERING_CORE_GL_TEXTURE_H_

#include <glad/glad.h>

class GLTexture {
public:
	GLTexture();
	virtual ~GLTexture();

	void Create();
	void Release();
	void Bind();

	// Upload texture data
	void Upload(int width, int height, int format, int type, const void* data);

	// Set filtering
	void SetFilter(int min_filter, int mag_filter);
	void SetWrap(int wrap_s, int wrap_t);

	GLuint GetID() const {
		return id;
	}
	bool IsCreated() const {
		return id != 0;
	}

private:
	GLuint id;
};

#endif
