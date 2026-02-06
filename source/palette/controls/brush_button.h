#ifndef RME_PALETTE_CONTROLS_BRUSH_BUTTON_H_
#define RME_PALETTE_CONTROLS_BRUSH_BUTTON_H_

#include "ui/controls/item_buttons.h"
#include "brushes/brush.h"

class BrushButton : public ItemToggleButton {
public:
	BrushButton(wxWindow* parent, Brush* brush, RenderSize sz, uint32_t id = wxID_ANY);
	BrushButton(wxWindow* parent, Brush* brush, RenderSize sz, EditorSprite* espr, uint32_t id = wxID_ANY);
	virtual ~BrushButton();

	Brush* brush;

	void OnKey(wxKeyEvent& event);
};

#endif
