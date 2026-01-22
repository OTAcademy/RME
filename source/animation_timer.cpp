//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "animation_timer.h"
#include "rendering/ui/map_display.h"

AnimationTimer::AnimationTimer(MapCanvas* canvas) :
	wxTimer(),
	map_canvas(canvas),
	started(false) {
		////
	};

AnimationTimer::~AnimationTimer() {
	////
};

void AnimationTimer::Notify() {
	if (map_canvas->GetZoom() <= 2.0) {
		map_canvas->Refresh();
	}
};

void AnimationTimer::Start() {
	if (!started) {
		started = true;
		wxTimer::Start(100);
	}
};

void AnimationTimer::Stop() {
	if (started) {
		started = false;
		wxTimer::Stop();
	}
};
