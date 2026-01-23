//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_UTILITIES_FRAME_PACER_H_
#define RME_RENDERING_UTILITIES_FRAME_PACER_H_

#include "rendering/utilities/fps_counter.h"

class FramePacer {
public:
	FramePacer();
	~FramePacer();

	void UpdateAndLimit(int limit, bool show_counter);

private:
	FPSCounter fps_counter;
};

#endif
