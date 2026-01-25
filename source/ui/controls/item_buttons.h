//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_ITEM_BUTTONS_H_
#define RME_ITEM_BUTTONS_H_

#include "app/main.h"
#include "ui/dcbutton.h"

/**
 * A toggle button with an item on it.
 */
class ItemToggleButton : public DCButton {
public:
	ItemToggleButton(wxWindow* parent, RenderSize size, int lookid, wxWindowID id = wxID_ANY) :
		DCButton(parent, id, wxDefaultPosition, DC_BTN_TOGGLE, size, lookid) { }
	virtual ~ItemToggleButton() { }
};

/**
 * A button with an item on it.
 */
class ItemButton : public DCButton {
public:
	ItemButton(wxWindow* parent, RenderSize size, uint16_t lookid, wxWindowID id = wxID_ANY) :
		DCButton(parent, id, wxDefaultPosition, DC_BTN_NORMAL, size, lookid) { }
	virtual ~ItemButton() { }
};

#endif
