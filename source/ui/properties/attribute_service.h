//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_ATTRIBUTE_SERVICE_H
#define RME_ATTRIBUTE_SERVICE_H

#include "app/main.h"
#include "game/item_attributes.h"
#include <wx/grid.h>

class Item;

class AttributeService {
public:
	static void setGridValue(wxGrid* grid, int rowIndex, const std::string& label, const ItemAttribute& attr);
	static void saveAttributesFromGrid(Item* item, wxGrid* grid);
	static ItemAttribute createFromTypeAndValue(const wxString& type, const wxString& value);
};

#endif
