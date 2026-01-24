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

#include "app/main.h"
#include "rendering/core/animator.h"
#include "ui/gui.h"

Animator::Animator(int frame_count, int start_frame, int loop_count, bool async) :
	frame_count(frame_count),
	start_frame(start_frame),
	loop_count(loop_count),
	async(async),
	current_frame(0),
	current_loop(0),
	current_duration(0),
	total_duration(0),
	direction(ANIMATION_FORWARD),
	last_time(0),
	is_complete(false) {
	ASSERT(start_frame >= -1 && start_frame < frame_count);

	for (int i = 0; i < frame_count; i++) {
		durations.push_back(newd FrameDuration(ITEM_FRAME_DURATION, ITEM_FRAME_DURATION));
	}

	reset();
}

Animator::~Animator() {
	for (int i = 0; i < frame_count; i++) {
		delete durations[i];
	}
	durations.clear();
}

int Animator::getStartFrame() const {
	if (start_frame > -1) {
		return start_frame;
	}
	return uniform_random(0, frame_count - 1);
}

FrameDuration* Animator::getFrameDuration(int frame) {
	ASSERT(frame >= 0 && frame < frame_count);
	return durations[frame];
}

int Animator::getFrame() {
	long time = g_gui.gfx.getElapsedTime();
	if (time != last_time && !is_complete) {
		long elapsed = time - last_time;
		if (elapsed >= current_duration) {
			int frame = 0;
			if (loop_count < 0) {
				frame = getPingPongFrame();
			} else {
				frame = getLoopFrame();
			}

			if (current_frame != frame) {
				int duration = getDuration(frame) - (elapsed - current_duration);
				if (duration < 0 && !async) {
					calculateSynchronous();
				} else {
					current_frame = frame;
					current_duration = std::max<int>(0, duration);
				}
			} else {
				is_complete = true;
			}
		} else {
			current_duration -= elapsed;
		}

		last_time = time;
	}
	return current_frame;
}

void Animator::setFrame(int frame) {
	ASSERT(frame == -1 || frame == 255 || frame == 254 || (frame >= 0 && frame < frame_count));

	if (current_frame == frame) {
		return;
	}

	if (async) {
		if (frame == 255) { // Async mode
			current_frame = 0;
		} else if (frame == 254) { // Random mode
			current_frame = uniform_random(0, frame_count - 1);
		} else if (frame >= 0 && frame < frame_count) {
			current_frame = frame;
		} else {
			current_frame = getStartFrame();
		}

		is_complete = false;
		last_time = g_gui.gfx.getElapsedTime();
		current_duration = getDuration(current_frame);
		current_loop = 0;
	} else {
		calculateSynchronous();
	}
}

void Animator::reset() {
	total_duration = 0;
	for (int i = 0; i < frame_count; i++) {
		total_duration += durations[i]->max;
	}

	is_complete = false;
	direction = ANIMATION_FORWARD;
	current_loop = 0;
	async = false;
	setFrame(-1);
}

int Animator::getDuration(int frame) const {
	ASSERT(frame >= 0 && frame < frame_count);
	return durations[frame]->getDuration();
}

int Animator::getPingPongFrame() {
	int count = direction == ANIMATION_FORWARD ? 1 : -1;
	int next_frame = current_frame + count;
	if (next_frame < 0 || next_frame >= frame_count) {
		direction = direction == ANIMATION_FORWARD ? ANIMATION_BACKWARD : ANIMATION_FORWARD;
		count *= -1;
	}
	return current_frame + count;
}

int Animator::getLoopFrame() {
	int next_phase = current_frame + 1;
	if (next_phase < frame_count) {
		return next_phase;
	}

	if (loop_count == 0) {
		return 0;
	}

	if (current_loop < (loop_count - 1)) {
		current_loop++;
		return 0;
	}
	return current_frame;
}

void Animator::calculateSynchronous() {
	long time = g_gui.gfx.getElapsedTime();
	if (time > 0 && total_duration > 0) {
		long elapsed = time % total_duration;
		int total_time = 0;
		for (int i = 0; i < frame_count; i++) {
			int duration = getDuration(i);
			if (elapsed >= total_time && elapsed < total_time + duration) {
				current_frame = i;
				current_duration = duration - (elapsed - total_time);
				break;
			}
			total_time += duration;
		}
		last_time = time;
	}
}
