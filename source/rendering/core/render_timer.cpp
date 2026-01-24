//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "rendering/core/render_timer.h"

RenderTimer::RenderTimer() {
	timer = std::make_unique<wxStopWatch>();
	timer->Start();
}

RenderTimer::~RenderTimer() {
}

void RenderTimer::Start() {
	timer->Start();
}

long RenderTimer::getElapsedTime() const {
	return (timer->TimeInMicro() / 1000).ToLong();
}
