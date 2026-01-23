//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_FPS_COUNTER_H_
#define RME_RENDERING_FPS_COUNTER_H_

#include <chrono>

#include <wx/string.h>

class FPSCounter {
public:
	FPSCounter();

	// Call this once per frame
	void Update();

	// Returns true if the FPS value changed this frame
	bool HasChanged() const;

	// Get the current FPS
	int GetFPS() const;

	// Get formatted status string
	wxString GetStatusString() const;

	// Limit the frame rate if needed (sleeps thread)
	void LimitFPS(int limit);

private:
	std::chrono::steady_clock::time_point last_frame_time;
	std::chrono::steady_clock::time_point last_fps_update;
	int frame_count;
	int current_fps;
	int last_displayed_fps;
	bool fps_changed;
};

#endif
