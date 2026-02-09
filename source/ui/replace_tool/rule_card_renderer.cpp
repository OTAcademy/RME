#include "app/main.h"
#include "ui/replace_tool/rule_card_renderer.h"
#include "util/nanovg_canvas.h"
#include "ui/replace_tool/rule_manager.h"
#include "game/items.h"
#include "util/nvg_utils.h"
#include <string>
#include <algorithm>
#include <format>

void RuleCardRenderer::DrawTrashIcon(NVGcontext* vg, float x, float y, float size, bool highlight) {
	nvgBeginPath(vg);
	float w = size * 0.6f;
	float h = size * 0.7f;
	float ox = x + (size - w) / 2;
	float oy = y + (size - h) / 2;

	nvgFillColor(vg, highlight ? nvgRGBA(255, 80, 80, 255) : nvgRGBA(200, 50, 50, 255));
	nvgRect(vg, ox, oy + 5, w, h - 5);
	nvgRect(vg, ox - 2, oy, w + 4, 3);
	nvgRect(vg, ox + w / 2 - 3, oy - 2, 6, 2);
	nvgFill(vg);
}

void RuleCardRenderer::DrawRuleItemCard(NanoVGCanvas* canvas, NVGcontext* vg, float x, float y, float w, float h, uint16_t id, bool highlight, bool isTrash, bool showDeleteOverlay, int probability) {
	NVGpaint bgPaint = nvgLinearGradient(vg, x, y, x, y + h, nvgRGBA(60, 60, 65, 255), nvgRGBA(50, 50, 55, 255));
	nvgBeginPath(vg);
	nvgRoundedRect(vg, x, y, w, h, 4.0f);
	nvgFillPaint(vg, bgPaint);
	nvgFill(vg);

	if (highlight) {
		nvgStrokeColor(vg, nvgRGBA(0, 120, 215, 255));
		nvgStrokeWidth(vg, 2.0f);
	} else {
		nvgStrokeColor(vg, nvgRGBA(60, 60, 70, 255));
		nvgStrokeWidth(vg, 1.0f);
	}
	nvgStroke(vg);

	float drawSize = 32.0f;

	if (isTrash) {
		DrawTrashIcon(vg, x + (w - drawSize) / 2, y + 8, drawSize, highlight);
		nvgFillColor(vg, nvgRGBA(200, 200, 200, 255));
		nvgFontSize(vg, 12.0f);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		nvgText(vg, x + w / 2, y + 44, "REMOVE", nullptr);
	} else if (id != 0) {
		// Use the canvas-provided caching mechanism to avoid leaks
		int tex = canvas->GetOrCreateItemImage(id);
		if (tex > 0) {
			int tw, th;
			nvgImageSize(vg, tex, &tw, &th);
			float scale = drawSize / std::max(tw, th);
			float dw = tw * scale;
			float dh = th * scale;
			float dx = x + (w - dw) / 2;
			float dy = y + 8 + (drawSize - dh) / 2;

			NVGpaint imgPaint = nvgImagePattern(vg, dx, dy, dw, dh, 0.0f, tex, 1.0f);
			nvgBeginPath(vg);
			nvgRect(vg, dx, dy, dw, dh);
			nvgFillPaint(vg, imgPaint);
			nvgFill(vg);
		}

		ItemType& it = g_items[id];
		std::string label = std::format("{} - {}", id, it.name);

		nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
		nvgFontSize(vg, 11.0f);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
		nvgTextBox(vg, x + 5, y + 44, w - 10, label.c_str(), nullptr);

		if (probability >= 0) {
			std::string probLabel = "Chance: " + std::to_string(probability) + "%";
			nvgFillColor(vg, nvgRGBA(160, 160, 160, 255));
			nvgFontSize(vg, 11.0f);
			nvgText(vg, x + w / 2, y + 84, probLabel.c_str(), nullptr);
		}
	}

	if (showDeleteOverlay) {
		float cx = x + w - 8;
		float cy = y + 8;
		float r = 8.0f;
		nvgBeginPath(vg);
		nvgCircle(vg, cx, cy, r);
		nvgFillColor(vg, nvgRGBA(200, 50, 50, 255));
		nvgFill(vg);
		nvgBeginPath(vg);
		float crossArr = r * 0.5f;
		nvgMoveTo(vg, cx - crossArr, cy - crossArr);
		nvgLineTo(vg, cx + crossArr, cy + crossArr);
		nvgMoveTo(vg, cx + crossArr, cy - crossArr);
		nvgLineTo(vg, cx - crossArr, cy + crossArr);
		nvgStrokeColor(vg, nvgRGBA(255, 255, 255, 255));
		nvgStrokeWidth(vg, 1.5f);
		nvgStroke(vg);
	}
}
