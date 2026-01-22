#include "rendering/screen_capture.h"
#include "main.h"

#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

void ScreenCapture::Capture(int width, int height, uint8_t* buffer) {
	glFinish(); // Wait for the operation to finish

	glPixelStorei(GL_PACK_ALIGNMENT, 1); // 1 byte alignment

	for (int i = 0; i < height; ++i) {
		glReadPixels(0, height - i, width, 1, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)(buffer) + 3 * width * i);
	}
}
