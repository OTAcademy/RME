//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "ui/properties/properties_window.h"

#include "ui/gui_ids.h"
#include "game/complexitem.h"
#include "ui/properties/container_properties_window.h"
#include "ui/properties/attribute_service.h"
#include "ui/properties/property_validator.h"
#include "ui/properties/property_applier.h"
#include "ui/properties/teleport_service.h"

#include <wx/grid.h>
#include <wx/wrapsizer.h>

PropertiesWindow::PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(parent, "Item Properties", map, tile_parent, item, pos),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	count_field(nullptr),
	tier_field(nullptr),
	currentPanel(nullptr) {
	ASSERT(edit_item);

	Bind(wxEVT_BUTTON, &PropertiesWindow::OnClickOK, this, wxID_OK);
	Bind(wxEVT_BUTTON, &PropertiesWindow::OnClickCancel, this, wxID_CANCEL);

	Bind(wxEVT_BUTTON, &PropertiesWindow::OnClickAddAttribute, this, ITEM_PROPERTIES_ADD_ATTRIBUTE);
	Bind(wxEVT_BUTTON, &PropertiesWindow::OnClickRemoveAttribute, this, ITEM_PROPERTIES_REMOVE_ATTRIBUTE);

	Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &PropertiesWindow::OnNotebookPageChanged, this, wxID_ANY);

	Bind(wxEVT_GRID_CELL_CHANGED, &PropertiesWindow::OnGridValueChanged, this);

	createUI();
}

void PropertiesWindow::createUI() {
	notebook = newd wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

	notebook->AddPage(createGeneralPanel(notebook), "Simple", true);
	if (dynamic_cast<Container*>(edit_item)) {
		notebook->AddPage(createContainerPanel(notebook), "Contents");
	}
	notebook->AddPage(createAttributesPanel(notebook), "Advanced");

	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);
	topSizer->Add(notebook, wxSizerFlags(1).DoubleBorder());

	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	wxButton* okBtn = newd wxButton(this, wxID_OK, "OK");
	okBtn->SetToolTip("Apply changes and close");
	optSizer->Add(okBtn, wxSizerFlags(0).Center());
	wxButton* cancelBtn = newd wxButton(this, wxID_CANCEL, "Cancel");
	cancelBtn->SetToolTip("Discard changes and close");
	optSizer->Add(cancelBtn, wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(topSizer);
	Centre(wxBOTH);
}

PropertiesWindow::~PropertiesWindow() {
	;
}

void PropertiesWindow::Update() {
	Container* container = dynamic_cast<Container*>(edit_item);
	if (container) {
		for (uint32_t i = 0; i < container->getVolume(); ++i) {
			container_items[i]->setItem(container->getItem(i));
		}
	}
	wxDialog::Update();
}

wxWindow* PropertiesWindow::createGeneralPanel(wxWindow* parent) {
	wxPanel* panel = newd wxPanel(parent, ITEM_PROPERTIES_GENERAL_TAB);
	wxFlexGridSizer* gridsizer = newd wxFlexGridSizer(2, 10, 10);
	gridsizer->AddGrowableCol(1);

	createGeneralFields(gridsizer, panel);
	createClassificationFields(gridsizer, panel);

	panel->SetSizerAndFit(gridsizer);
	return panel;
}

void PropertiesWindow::createGeneralFields(wxFlexGridSizer* gridsizer, wxWindow* panel) {
	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "ID " + i2ws(edit_item->getID())));
	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, (edit_item->isCharged() ? "Charges" : "Count")));
	int max_count = 100;
	if (edit_item->isClientCharged()) {
		max_count = 250;
	}
	if (edit_item->isExtraCharged()) {
		max_count = 65500;
	}
	count_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getCount()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, max_count, edit_item->getCount());
	count_field->SetToolTip("Number of items in stack / Charges");
	if (!edit_item->isStackable() && !edit_item->isCharged()) {
		count_field->Enable(false);
		count_field->SetToolTip("Only stackable or charged items can have a count.");
	}
	gridsizer->Add(count_field, wxSizerFlags(1).Expand());

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Action ID"));
	action_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
	action_id_field->SetToolTip("Action ID (0-65535). Used for scripting.");
	gridsizer->Add(action_id_field, wxSizerFlags(1).Expand());

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Unique ID"));
	unique_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
	unique_id_field->SetToolTip("Unique ID (0-65535). Must be unique on the map.");
	gridsizer->Add(unique_id_field, wxSizerFlags(1).Expand());
}

void PropertiesWindow::createClassificationFields(wxFlexGridSizer* gridsizer, wxWindow* panel) {
	if (g_items.MajorVersion >= 3 && g_items.MinorVersion >= 60 && (edit_item->getClassification() > 0 || edit_item->isWeapon() || edit_item->isWearableEquipment())) {
		gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Classification"));
		gridsizer->Add(newd wxStaticText(panel, wxID_ANY, i2ws(edit_item->getClassification())));

		gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Tier"));
		tier_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getTier()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFF, edit_item->getTier());
		tier_field->SetToolTip("Item tier (0-255)");
		gridsizer->Add(tier_field, wxSizerFlags(1).Expand());
	}
}

wxWindow* PropertiesWindow::createContainerPanel(wxWindow* parent) {
	Container* container = (Container*)edit_item;
	wxPanel* panel = newd wxPanel(parent, ITEM_PROPERTIES_CONTAINER_TAB);
	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* gridSizer = newd wxWrapSizer(wxHORIZONTAL);

	bool use_large_sprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
	for (uint32_t i = 0; i < container->getVolume(); ++i) {
		Item* item = container->getItem(i);
		ContainerItemButton* containerItemButton = newd ContainerItemButton(panel, use_large_sprites, i, edit_map, item);

		container_items.push_back(containerItemButton);
		gridSizer->Add(containerItemButton, wxSizerFlags(0).Border(wxALL, 2));
	}

	topSizer->Add(gridSizer, wxSizerFlags(1).Expand());

	/*
	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_ADD_ATTRIBUTE, "Add Item"), wxSizerFlags(0).Center());
	// optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_REMOVE_ATTRIBUTE, "Remove Attribute"), wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());
	*/

	panel->SetSizer(topSizer);
	return panel;
}

wxWindow* PropertiesWindow::createAttributesPanel(wxWindow* parent) {
	wxPanel* panel = newd wxPanel(parent, wxID_ANY);
	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);

	attributesGrid = newd wxGrid(panel, ITEM_PROPERTIES_ADVANCED_TAB, wxDefaultPosition, wxSize(-1, FromDIP(160)));
	topSizer->Add(attributesGrid, wxSizerFlags(1).Expand());

	wxFont time_font(*wxSWISS_FONT);
	attributesGrid->SetDefaultCellFont(time_font);
	attributesGrid->CreateGrid(0, 3);
	attributesGrid->DisableDragRowSize();
	attributesGrid->DisableDragColSize();
	attributesGrid->SetSelectionMode(wxGrid::wxGridSelectRows);
	attributesGrid->SetRowLabelSize(0);
	// log->SetColLabelSize(0);
	// log->EnableGridLines(false);
	attributesGrid->EnableEditing(true);

	attributesGrid->SetColLabelValue(0, "Key");
	attributesGrid->SetColSize(0, FromDIP(100));
	attributesGrid->SetColLabelValue(1, "Type");
	attributesGrid->SetColSize(1, FromDIP(80));
	attributesGrid->SetColLabelValue(2, "Value");
	attributesGrid->SetColSize(2, FromDIP(410));

	// contents
	ItemAttributeMap attrs = edit_item->getAttributes();
	attributesGrid->AppendRows(attrs.size());
	int i = 0;
	for (ItemAttributeMap::iterator aiter = attrs.begin(); aiter != attrs.end(); ++aiter, ++i) {
		AttributeService::setGridValue(attributesGrid, i, aiter->first, aiter->second);
	}

	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	wxButton* addBtn = newd wxButton(panel, ITEM_PROPERTIES_ADD_ATTRIBUTE, "Add Attribute");
	addBtn->SetToolTip("Add a new custom attribute");
	optSizer->Add(addBtn, wxSizerFlags(0).Center());

	wxButton* removeBtn = newd wxButton(panel, ITEM_PROPERTIES_REMOVE_ATTRIBUTE, "Remove Attribute");
	removeBtn->SetToolTip("Remove selected custom attribute");
	optSizer->Add(removeBtn, wxSizerFlags(0).Center());

	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());

	panel->SetSizer(topSizer);

	return panel;
}

void PropertiesWindow::SetGridValue(wxGrid* grid, int rowIndex, std::string label, const ItemAttribute& attr) {
	AttributeService::setGridValue(grid, rowIndex, label, attr);
}

void PropertiesWindow::OnResize(wxSizeEvent& evt) {
	/*
	if(wxGrid* grid = (wxGrid*)currentPanel->FindWindowByName("AdvancedGrid")) {
		int tWidth = 0;
		for(int i = 0; i < 3; ++i)
			tWidth += grid->GetColumnWidth(i);

		int wWidth = grid->GetParent()->GetSize().GetWidth();

		grid->SetColumnWidth(2, wWidth - 100 - 80);
	}
	*/
}

void PropertiesWindow::OnNotebookPageChanged(wxNotebookEvent& evt) {
	wxWindow* page = notebook->GetCurrentPage();

	// TODO: Save

	switch (page->GetId()) {
		case ITEM_PROPERTIES_GENERAL_TAB: {
			// currentPanel = createGeneralPanel(page);
			break;
		}
		case ITEM_PROPERTIES_ADVANCED_TAB: {
			// currentPanel = createAttributesPanel(page);
			break;
		}
		default:
			break;
	}
}

void PropertiesWindow::saveGeneralPanel() {
	if (edit_item) {
		int new_uid = unique_id_field->GetValue();
		int new_aid = action_id_field->GetValue();
		int new_count = count_field->GetValue();
		int new_tier = (tier_field ? tier_field->GetValue() : 0);

		PropertyApplier::applyItemProperties(edit_item, new_count, new_uid, new_aid, new_tier);
	}
}

void PropertiesWindow::saveContainerPanel() {
	////
}

void PropertiesWindow::saveAttributesPanel() {
	AttributeService::saveAttributesFromGrid(edit_item, attributesGrid);
}

void PropertiesWindow::OnGridValueChanged(wxGridEvent& event) {
	if (event.GetCol() == 1) {
		wxString newType = attributesGrid->GetCellValue(event.GetRow(), 1);
		if (newType == event.GetString()) {
			return;
		}

		ItemAttribute attr = AttributeService::createFromTypeAndValue(newType, attributesGrid->GetCellValue(event.GetRow(), 2));
		AttributeService::setGridValue(attributesGrid, event.GetRow(), nstr(attributesGrid->GetCellValue(event.GetRow(), 0)), attr);
	}
}

void PropertiesWindow::OnClickOK(wxCommandEvent&) {
	int new_uid = unique_id_field->GetValue();
	int new_aid = action_id_field->GetValue();
	int new_tier = (tier_field ? tier_field->GetValue() : 0);

	if (!PropertyValidator::validateItemProperties(this, new_uid, new_aid, new_tier)) {
		return;
	}

	saveGeneralPanel();
	saveAttributesPanel();
	EndModal(1);
}

void PropertiesWindow::OnClickAddAttribute(wxCommandEvent&) {
	attributesGrid->AppendRows(1);
	ItemAttribute attr(0);
	SetGridValue(attributesGrid, attributesGrid->GetNumberRows() - 1, "", attr);
}

void PropertiesWindow::OnClickRemoveAttribute(wxCommandEvent&) {
	const auto rowIndexes = attributesGrid->GetSelectedRows();
	if (rowIndexes.Count() != 1) {
		return;
	}

	int rowIndex = rowIndexes[0];
	attributesGrid->DeleteRows(rowIndex, 1);
}

void PropertiesWindow::OnClickCancel(wxCommandEvent&) {
	EndModal(0);
}
