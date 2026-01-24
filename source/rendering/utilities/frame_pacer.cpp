//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "rendering/utilities/frame_pacer.h"
#include "rendering/ui/map_status_updater.h"

FramePacer::FramePacer() {
}

FramePacer::~FramePacer() {
}

void FramePacer::UpdateAndLimit(int limit, bool show_counter) {
	fps_counter.LimitFPS(limit);
	fps_counter.Update();

	if (show_counter && fps_counter.HasChanged()) {
		MapStatusUpdater::UpdateFPS(fps_counter.GetStatusString());
	}
}
