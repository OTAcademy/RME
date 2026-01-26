//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/attribute_service.h"
#include "game/item.h"

void AttributeService::setGridValue(wxGrid* grid, int rowIndex, const std::string& label, const ItemAttribute& attr) {
	wxArrayString types;
	types.Add("Number");
	types.Add("Float");
	types.Add("Boolean");
	types.Add("String");

	grid->SetCellValue(rowIndex, 0, label);
	switch (attr.type) {
		case ItemAttribute::STRING: {
			grid->SetCellValue(rowIndex, 1, "String");
			grid->SetCellValue(rowIndex, 2, wxstr(*attr.getString()));
			break;
		}
		case ItemAttribute::INTEGER: {
			grid->SetCellValue(rowIndex, 1, "Number");
			grid->SetCellValue(rowIndex, 2, i2ws(*attr.getInteger()));
			grid->SetCellEditor(rowIndex, 2, new wxGridCellNumberEditor);
			break;
		}
		case ItemAttribute::DOUBLE:
		case ItemAttribute::FLOAT: {
			grid->SetCellValue(rowIndex, 1, "Float");
			wxString f;
			f << *attr.getFloat();
			grid->SetCellValue(rowIndex, 2, f);
			grid->SetCellEditor(rowIndex, 2, new wxGridCellFloatEditor);
			break;
		}
		case ItemAttribute::BOOLEAN: {
			grid->SetCellValue(rowIndex, 1, "Boolean");
			grid->SetCellValue(rowIndex, 2, *attr.getBoolean() ? "1" : "");
			grid->SetCellRenderer(rowIndex, 2, new wxGridCellBoolRenderer);
			grid->SetCellEditor(rowIndex, 2, new wxGridCellBoolEditor);
			break;
		}
		default: {
			grid->SetCellValue(rowIndex, 1, "Unknown");
			grid->SetCellBackgroundColour(rowIndex, 1, *wxLIGHT_GREY);
			grid->SetCellBackgroundColour(rowIndex, 2, *wxLIGHT_GREY);
			grid->SetReadOnly(rowIndex, 1, true);
			grid->SetReadOnly(rowIndex, 2, true);
			break;
		}
	}
	grid->SetCellEditor(rowIndex, 1, new wxGridCellChoiceEditor(types));
}

void AttributeService::saveAttributesFromGrid(Item* item, wxGrid* grid) {
	item->clearAllAttributes();
	for (int32_t rowIndex = 0; rowIndex < grid->GetNumberRows(); ++rowIndex) {
		wxString label = grid->GetCellValue(rowIndex, 0);
		if (label.IsEmpty()) {
			continue;
		}

		ItemAttribute attr = createFromTypeAndValue(grid->GetCellValue(rowIndex, 1), grid->GetCellValue(rowIndex, 2));
		if (attr.type != ItemAttribute::NONE) {
			item->setAttribute(nstr(label), attr);
		}
	}
}

ItemAttribute AttributeService::createFromTypeAndValue(const wxString& type, const wxString& value) {
	ItemAttribute attr;
	if (type == "String") {
		attr.set(nstr(value));
	} else if (type == "Float") {
		double dval;
		if (value.ToDouble(&dval)) {
			attr.set(dval);
		}
	} else if (type == "Number") {
		long lval;
		if (value.ToLong(&lval)) {
			attr.set(static_cast<int32_t>(lval));
		}
	} else if (type == "Boolean") {
		attr.set(value == "1");
	}
	return attr;
}
