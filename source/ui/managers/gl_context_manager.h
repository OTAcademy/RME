#ifndef RME_GL_CONTEXT_MANAGER_H_
#define RME_GL_CONTEXT_MANAGER_H_

#include <wx/wx.h>
#include <wx/glcanvas.h>

class GLContextManager {
public:
	GLContextManager();
	~GLContextManager();

	wxGLContext* GetGLContext(wxGLCanvas* win);

private:
	wxGLContext* OGLContext;
};

extern GLContextManager g_gl_context;

#endif
