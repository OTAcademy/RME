//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_ANIMATOR_H_
#define RME_ANIMATOR_H_

#include <vector>
#include "util/common.h"

enum AnimationDirection {
	ANIMATION_FORWARD = 0,
	ANIMATION_BACKWARD = 1
};

enum ItemAnimationDuration {
	ITEM_FRAME_DURATION = 500
};

struct FrameDuration {
	int min;
	int max;

	FrameDuration(int min, int max) :
		min(min), max(max) {
		ASSERT(min <= max);
	}

	int getDuration() const {
		if (min == max) {
			return min;
		}
		return uniform_random(min, max);
	};

	void setValues(int min, int max) {
		ASSERT(min <= max);
		this->min = min;
		this->max = max;
	}
};

class Animator {
public:
	Animator(int frames, int start_frame, int loop_count, bool async);
	~Animator();

	int getStartFrame() const;

	FrameDuration* getFrameDuration(int frame);

	int getFrame();
	void setFrame(int frame);

	void reset();

private:
	int getDuration(int frame) const;
	int getPingPongFrame();
	int getLoopFrame();
	void calculateSynchronous();

	int frame_count;
	int start_frame;
	int loop_count;
	bool async;
	std::vector<FrameDuration> durations;
	int current_frame;
	int current_loop;
	int current_duration;
	int total_duration;
	AnimationDirection direction;
	long last_time;
	bool is_complete;
};

#endif
