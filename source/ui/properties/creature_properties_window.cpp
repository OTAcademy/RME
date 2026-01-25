//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/properties/creature_properties_window.h"

#include "game/creature.h"
#include "ui/gui.h"

BEGIN_EVENT_TABLE(CreaturePropertiesWindow, wxDialog)
EVT_BUTTON(wxID_OK, CreaturePropertiesWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, CreaturePropertiesWindow::OnClickCancel)
END_EVENT_TABLE()

CreaturePropertiesWindow::CreaturePropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Creature* creature, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Creature Properties", map, tile_parent, creature, pos),
	count_field(nullptr),
	direction_field(nullptr) {
	ASSERT(edit_creature);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Creature Properties");

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Creature "));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_creature->getName()) + "\""), wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn interval"));
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_creature->getSpawnTime()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 86400, edit_creature->getSpawnTime());
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Direction"));
	direction_field = newd wxChoice(this, wxID_ANY);

	for (Direction dir = DIRECTION_FIRST; dir <= DIRECTION_LAST; ++dir) {
		direction_field->Append(wxstr(Creature::DirID2Name(dir)), newd int32_t(dir));
	}
	direction_field->SetSelection(edit_creature->getDirection());
	subsizer->Add(direction_field, wxSizerFlags(1).Expand());

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(3).Expand().Border(wxALL, 20));

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

CreaturePropertiesWindow::~CreaturePropertiesWindow() {
	if (direction_field) {
		for (uint32_t i = 0; i < direction_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(direction_field->GetClientData(i));
		}
	}
}

void CreaturePropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	int new_spawntime = count_field->GetValue();
	edit_creature->setSpawnTime(new_spawntime);

	int* new_dir = reinterpret_cast<int*>(direction_field->GetClientData(
		direction_field->GetSelection()
	));

	if (new_dir) {
		edit_creature->setDirection((Direction)*new_dir);
	}
	EndModal(1);
}

void CreaturePropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}
