//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "rendering/drawers/entities/creature_name_drawer.h"
#include "rendering/core/render_view.h"
#include "rendering/core/text_renderer.h"
#include <nanovg.h>
#include "rendering/core/coordinate_mapper.h"
#include "game/creature.h"

CreatureNameDrawer::CreatureNameDrawer() {
}

CreatureNameDrawer::~CreatureNameDrawer() {
	clear();
}

void CreatureNameDrawer::clear() {
	labels.clear();
}

void CreatureNameDrawer::addLabel(const Position& pos, const std::string& name, const Creature* c) {
	if (name.empty()) {
		return;
	}
	labels.push_back({ pos, name, c });
}

void CreatureNameDrawer::draw(NVGcontext* vg, const RenderView& view) {
	if (!vg) {
		return;
	}

	if (labels.empty()) {
		return;
	}

	float zoom = view.zoom;
	float tile_size_screen = 32.0f / zoom;
	float fontSize = 11.0f; // Original size or slightly larger if preferred, reverting to 11.0f

	nvgFontSize(vg, fontSize);
	nvgFontFace(vg, "sans");
	nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_BOTTOM);

	for (const auto& label : labels) {
		if (label.pos.z != view.camera_pos.z) {
			continue;
		}

		int unscaled_x, unscaled_y;
		view.getScreenPosition(label.pos.x, label.pos.y, label.pos.z, unscaled_x, unscaled_y);

		float screen_x = (float)unscaled_x / zoom;
		float screen_y = (float)unscaled_y / zoom;

		// Center on tile, position slightly above the creature head
		// Standard creature is 32x32, but might be tall.
		// Safest is to anchor to the tile top.
		float labelX = screen_x + tile_size_screen / 2.0f;
		float labelY = screen_y - 2.0f; // slight gap above tile top

		float textBounds[4];
		nvgTextBounds(vg, 0, 0, label.name.c_str(), nullptr, textBounds);
		float textWidth = textBounds[2] - textBounds[0];
		float textHeight = textBounds[3] - textBounds[1];

		float paddingX = 4.0f;
		float paddingY = 2.0f;

		// Draw background (Black transparent)
		nvgBeginPath(vg);
		nvgRoundedRect(vg, labelX - textWidth / 2.0f - paddingX, labelY - textHeight - paddingY * 2.0f, textWidth + paddingX * 2.0f, textHeight + paddingY * 2.0f, 3.0f);
		nvgFillColor(vg, nvgRGBA(0, 0, 0, 160)); // Transparent black
		nvgFill(vg);

		// Draw Text (White)
		nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
		nvgText(vg, labelX, labelY - paddingY, label.name.c_str(), nullptr);
	}
}
