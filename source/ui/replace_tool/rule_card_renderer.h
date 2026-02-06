#ifndef RME_RULE_CARD_RENDERER_H_
#define RME_RULE_CARD_RENDERER_H_

#include <nanovg.h>
#include <cstdint>

class RuleCardRenderer {
public:
	static void DrawRuleItemCard(NVGcontext* vg, float x, float y, float w, float h, uint16_t id, bool highlight, bool isTrash, bool showDeleteOverlay, int probability = -1);
	static void DrawTrashIcon(NVGcontext* vg, float x, float y, float size, bool highlight);
};

#endif
