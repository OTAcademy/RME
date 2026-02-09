#ifndef RME_RULE_CARD_RENDERER_H_
#define RME_RULE_CARD_RENDERER_H_

#include <nanovg.h>
#include <cstdint>

class NanoVGCanvas;

class RuleBuilderPanel;
struct ReplacementRule;

class RuleCardRenderer {
public:
	// Layout Constants (Shared)
	static const int CARD_PADDING = 20;
	static const int CARD_MARGIN_X = 10;
	static const int CARD_MARGIN_Y = 10;
	static const int HEADER_HEIGHT = 40;
	static const int ITEM_SIZE = 56;
	static const int ITEM_H = 110;
	static const int ITEM_SPACING = 10;
	static const int ARROW_WIDTH = 60;
	static const int SECTION_GAP = 20;
	static const int CARD_W; // ITEM_SIZE + 20

	static void DrawRuleItemCard(NanoVGCanvas* canvas, NVGcontext* vg, float x, float y, float w, float h, uint16_t id, bool highlight, bool isTrash, bool showDeleteOverlay, int probability = -1);
	static void DrawTrashIcon(NVGcontext* vg, float x, float y, float size, bool highlight);

	// New Static Helper Methods for RuleBuilderPanel
	static void DrawHeader(NVGcontext* vg, float width);
	static void DrawClearButton(NVGcontext* vg, float width, bool isHovered);
	static void DrawSaveButton(NVGcontext* vg, float width, bool isHovered);
	static void DrawRuleCard(RuleBuilderPanel* panel, NVGcontext* vg, int ruleIndex, int y, int width, bool hoverDelete, int dragHoverTargetIdx, int dragHoverType);
	static void DrawRuleArrow(NVGcontext* vg, float x, float y, float h);
	static void DrawNewRuleArea(NVGcontext* vg, float width, float y, bool isHovered);
};

#endif
