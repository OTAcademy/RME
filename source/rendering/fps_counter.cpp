//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "rendering/fps_counter.h"
#include <thread>

FPSCounter::FPSCounter() :
	frame_count(0),
	current_fps(0),
	last_displayed_fps(-1),
	fps_changed(false) {
	last_frame_time = std::chrono::steady_clock::now();
	last_fps_update = std::chrono::steady_clock::now();
}

void FPSCounter::Update() {
	auto now = std::chrono::steady_clock::now();
	frame_count++;

	fps_changed = false;

	// Update FPS counter every second
	auto fps_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fps_update);
	if (fps_elapsed.count() >= 1000) {
		current_fps = frame_count;
		frame_count = 0;
		last_fps_update = now;

		if (current_fps != last_displayed_fps) {
			last_displayed_fps = current_fps;
			fps_changed = true;
		}
	}
}

bool FPSCounter::HasChanged() const {
	return fps_changed;
}

int FPSCounter::GetFPS() const {
	return current_fps;
}

void FPSCounter::LimitFPS(int limit) {
	if (limit <= 0) {
		return;
	}

	auto now = std::chrono::steady_clock::now();
	auto target_frame_duration = std::chrono::microseconds(1000000 / limit);
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_frame_time);

	if (elapsed < target_frame_duration) {
		std::this_thread::sleep_for(target_frame_duration - elapsed);
	}
	last_frame_time = std::chrono::steady_clock::now();
}
