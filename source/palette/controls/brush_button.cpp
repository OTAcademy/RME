#include "app/main.h"
#include "palette/controls/brush_button.h"
#include "ui/gui.h"
#include "game/sprites.h"
#include "rendering/core/editor_sprite.h"

// ============================================================================
// Brush Button

BEGIN_EVENT_TABLE(BrushButton, ItemToggleButton)
EVT_KEY_DOWN(BrushButton::OnKey)
END_EVENT_TABLE()

BrushButton::BrushButton(wxWindow* parent, Brush* _brush, RenderSize sz, uint32_t id) :
	ItemToggleButton(parent, sz, uint16_t(0), id),
	brush(_brush) {
	ASSERT(sz != RENDER_SIZE_64x64);
	ASSERT(brush);
	SetSprite(brush->getLookID());
	SetToolTip(wxstr(brush->getName()));
}

BrushButton::BrushButton(wxWindow* parent, Brush* _brush, RenderSize sz, EditorSprite* espr, uint32_t id) :
	ItemToggleButton(parent, sz, uint16_t(0), id),
	brush(_brush) {
	ASSERT(sz != RENDER_SIZE_64x64);
	ASSERT(brush);
	SetSprite(espr);
	SetToolTip(wxstr(brush->getName()));
}

BrushButton::~BrushButton() {
	////
}

void BrushButton::OnKey(wxKeyEvent& event) {
	g_gui.AddPendingCanvasEvent(event);
}
