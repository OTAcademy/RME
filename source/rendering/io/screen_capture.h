#ifndef RME_RENDERING_SCREEN_CAPTURE_H_
#define RME_RENDERING_SCREEN_CAPTURE_H_

#include <cstdint>

class ScreenCapture {
public:
	static void Capture(int width, int height, uint8_t* buffer);
};

#endif
