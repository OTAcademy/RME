//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_ANIMATION_TIMER_H_
#define RME_ANIMATION_TIMER_H_

#include <wx/wx.h>
#include <wx/timer.h>

class MapCanvas;

class AnimationTimer : public wxTimer {
public:
	AnimationTimer(MapCanvas* canvas);
	~AnimationTimer();

	void Notify();
	void Start();
	void Stop();

private:
	MapCanvas* map_canvas;
	bool started;
};

#endif
