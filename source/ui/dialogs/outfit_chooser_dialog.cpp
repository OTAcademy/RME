#include "ui/dialogs/outfit_chooser_dialog.h"
#include <wx/statline.h>
#include <wx/sizer.h>
#include <wx/dcclient.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/scrolwin.h>
#include <wx/msgdlg.h>
#include <random>
#include <wx/artprov.h>
#include <wx/graphics.h>
#include <wx/dcbuffer.h>
#include <wx/settings.h>
#include <wx/spinctrl.h>
#include <wx/textdlg.h>
#include <wx/stdpaths.h>
#include <wx/menu.h>

#include "game/creatures.h"
#include "rendering/drawers/entities/creature_drawer.h"
#include "rendering/utilities/sprite_icon_generator.h"
#include "rendering/core/graphics.h"
#include "rendering/core/outfit_colors.h"
#include "rendering/core/outfit_colorizer.h"
#include "ui/gui.h"
#include "util/json.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace {
	const int COLOR_COLUMNS = 19;
	const int COLOR_ROWS = 7;
	const int PREVIEW_SIZE = 192;
	const int OUTFIT_TILE_WIDTH = 100;
	const int OUTFIT_TILE_HEIGHT = 120;
}

// ============================================================================
// OutfitChooserDialog
// ============================================================================

OutfitChooserDialog::OutfitChooserDialog(wxWindow* parent, const Outfit& current_outfit) :
	wxDialog(parent, wxID_ANY, "Customise Character", wxDefaultPosition, wxSize(1200, 850), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
	current_outfit(current_outfit),
	current_speed(220),
	current_name("You"),
	selected_color_part(0) {

	selection_panel = new OutfitSelectionGrid(this, this, false);
	favorites_panel = new OutfitSelectionGrid(this, this, true);

	// Load preferences on startup
	g_preview_preferences.load();
	current_speed = g_preview_preferences.getSpeed();
	current_name = wxString::FromUTF8(g_preview_preferences.getName());
	favorites = g_preview_preferences.getFavorites();

	// Resolve names from g_creatures
	std::map<int, wxString> looktype_to_name;
	for (auto it = g_creatures.begin(); it != g_creatures.end(); ++it) {
		CreatureType* ct = it->second;
		if (ct && ct->outfit.lookType != 0) {
			if (looktype_to_name.find(ct->outfit.lookType) == looktype_to_name.end()) {
				looktype_to_name[ct->outfit.lookType] = wxstr(ct->name);
			}
		}
	}

	int maxLookType = g_gui.gfx.getCreatureSpriteMaxID();
	for (int i = 1; i <= maxLookType; ++i) {
		OutfitItem item;
		item.lookType = i;
		auto it = looktype_to_name.find(i);
		if (it != looktype_to_name.end()) {
			item.name = it->second;
		} else {
			item.name = wxString::Format("Outfit %d", i);
		}
		selection_panel->all_outfits.push_back(item);
	}
	selection_panel->UpdateFilter("");
	favorites_panel->favorite_items = favorites;
	favorites_panel->UpdateVirtualSize();

	wxBoxSizer* outer_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);

	auto CreateHeader = [this](const wxString& title) -> wxWindow* {
		wxStaticText* header = new wxStaticText(this, wxID_ANY, title);
		wxFont font = header->GetFont();
		font.SetWeight(wxFONTWEIGHT_BOLD);
		font.SetPointSize(font.GetPointSize() + 1);
		header->SetFont(font);
		header->SetForegroundColour(wxColour(110, 110, 110)); // Subtle grey for headers
		return header;
	};

	// Column 1: Preview & Appearance (The Hero column)
	wxBoxSizer* col1_sizer = new wxBoxSizer(wxVERTICAL);

	col1_sizer->Add(CreateHeader("Character Preview"), 0, wxLEFT | wxTOP, 8);
	preview_panel = new OutfitPreviewPanel(this, current_outfit);
	col1_sizer->Add(preview_panel, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 12);

	col1_sizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);

	col1_sizer->Add(CreateHeader("Appearance"), 0, wxLEFT | wxTOP, 8);
	wxBoxSizer* part_sizer = new wxBoxSizer(wxHORIZONTAL);
	head_btn = new wxButton(this, ID_COLOR_HEAD, "Head", wxDefaultPosition, wxSize(50, -1));
	body_btn = new wxButton(this, ID_COLOR_BODY, "Primary", wxDefaultPosition, wxSize(50, -1));
	legs_btn = new wxButton(this, ID_COLOR_LEGS, "Secondary", wxDefaultPosition, wxSize(50, -1));
	feet_btn = new wxButton(this, ID_COLOR_FEET, "Detail", wxDefaultPosition, wxSize(50, -1));

	part_sizer->Add(head_btn, 1, wxEXPAND | wxRIGHT, 2);
	part_sizer->Add(body_btn, 1, wxEXPAND | wxRIGHT, 2);
	part_sizer->Add(legs_btn, 1, wxEXPAND | wxRIGHT, 2);
	part_sizer->Add(feet_btn, 1, wxEXPAND);
	col1_sizer->Add(part_sizer, 0, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 8);

	wxGridSizer* palette_sizer = new wxGridSizer(COLOR_ROWS, COLOR_COLUMNS, 1, 1);
	for (int i = 0; i < (int)TemplateOutfitLookupTableSize; ++i) {
		uint32_t color = TemplateOutfitLookupTable[i];
		wxButton* btn = new wxButton(this, ID_COLOR_START + i, "", wxDefaultPosition, wxSize(16, 16), wxBORDER_NONE);
		btn->SetBackgroundColour(wxColour((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF));
		palette_sizer->Add(btn);
		color_buttons.push_back(btn);

		btn->Bind(wxEVT_BUTTON, [this, i](wxCommandEvent&) {
			SelectColor(i);
		});
	}
	col1_sizer->Add(palette_sizer, 0, wxALIGN_CENTER | wxALL, 8);

	col1_sizer->Add(CreateHeader("Configuration"), 0, wxLEFT | wxTOP, 8);
	wxGridSizer* check_sizer = new wxGridSizer(1, 4, 2, 2);
	addon1 = new wxCheckBox(this, ID_ADDON1, "Addon 1");
	addon1->SetValue((current_outfit.lookAddon & 1) != 0);
	addon2 = new wxCheckBox(this, ID_ADDON2, "Addon 2");
	addon2->SetValue((current_outfit.lookAddon & 2) != 0);

	check_sizer->Add(addon1, 0, wxALL, 2);
	check_sizer->Add(addon2, 0, wxALL, 2);

	wxButton* rand_btn = new wxButton(this, ID_RANDOMIZE, "Random", wxDefaultPosition, wxSize(60, 22));
	check_sizer->Add(rand_btn, 0, wxLEFT, 5);

	col1_sizer->Add(check_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 8);

	wxFlexGridSizer* text_fields = new wxFlexGridSizer(2, 2, 4, 8);
	text_fields->Add(new wxStaticText(this, wxID_ANY, "Speed:"), 0, wxALIGN_CENTER_VERTICAL);
	speed_ctrl = new wxSpinCtrl(this, ID_SPEED, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 50, 3000, current_speed);
	text_fields->Add(speed_ctrl, 1, wxEXPAND);

	text_fields->Add(new wxStaticText(this, wxID_ANY, "Name:"), 0, wxALIGN_CENTER_VERTICAL);
	name_ctrl = new wxTextCtrl(this, ID_NAME, current_name);
	text_fields->Add(name_ctrl, 1, wxEXPAND);
	text_fields->AddGrowableCol(1);
	col1_sizer->Add(text_fields, 0, wxEXPAND | wxALL, 8);

	main_sizer->Add(col1_sizer, 0, wxEXPAND | wxRIGHT, 10);

	// Column 2: Available Outfits
	wxBoxSizer* outfits_sizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* outfits_header_row = new wxBoxSizer(wxHORIZONTAL);
	outfits_header_row->Add(CreateHeader("Available Outfits"), 1, wxALIGN_BOTTOM | wxLEFT, 4);

	outfit_search = new wxSearchCtrl(this, ID_SEARCH, wxEmptyString, wxDefaultPosition, wxSize(180, -1));
	outfit_search->SetDescriptiveText("Search...");
	outfits_header_row->Add(outfit_search, 0, wxALIGN_BOTTOM | wxRIGHT, 4);

	outfits_sizer->Add(outfits_header_row, 0, wxEXPAND | wxBOTTOM, 4);
	outfits_sizer->Add(selection_panel, 1, wxEXPAND);
	main_sizer->Add(outfits_sizer, 1, wxEXPAND | wxLEFT | wxRIGHT, 5);

	// Column 3: Favorites
	wxBoxSizer* favs_sizer = new wxBoxSizer(wxVERTICAL);
	favs_sizer->Add(CreateHeader("Favorites"), 0, wxBOTTOM | wxLEFT, 4);

	favorites_panel->SetMinSize(wxSize(430, -1)); // Force width for 4 columns of 100px
	favs_sizer->Add(favorites_panel, 1, wxEXPAND);

	wxButton* fav_btn = new wxButton(this, ID_ADD_FAVORITE, "Save Current Outfit as Favorite", wxDefaultPosition, wxSize(-1, 28));
	favs_sizer->Add(fav_btn, 0, wxEXPAND | wxTOP, 8);

	main_sizer->Add(favs_sizer, 0, wxEXPAND | wxLEFT, 5);

	outer_sizer->Add(main_sizer, 1, wxEXPAND | wxALL, 10);

	// Bottom Bar (Actions)
	wxStaticLine* bottom_line = new wxStaticLine(this, wxID_ANY);
	outer_sizer->Add(bottom_line, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	wxBoxSizer* bottom_sizer = new wxBoxSizer(wxHORIZONTAL);
	bottom_sizer->AddStretchSpacer();

	wxButton* ok_btn = new wxButton(this, wxID_OK, "OK", wxDefaultPosition, wxSize(90, 30));
	ok_btn->SetToolTip("Confirm outfit selection");
	wxButton* cancel_btn = new wxButton(this, wxID_CANCEL, "Cancel", wxDefaultPosition, wxSize(90, 30));
	cancel_btn->SetToolTip("Cancel outfit selection");
	bottom_sizer->Add(ok_btn, 0, wxALL, 8);
	bottom_sizer->Add(cancel_btn, 0, wxALL, 8);

	outer_sizer->Add(bottom_sizer, 0, wxEXPAND);

	SetSizer(outer_sizer);

	// Bind Events
	head_btn->Bind(wxEVT_BUTTON, &OutfitChooserDialog::OnColorPartChange, this);
	body_btn->Bind(wxEVT_BUTTON, &OutfitChooserDialog::OnColorPartChange, this);
	legs_btn->Bind(wxEVT_BUTTON, &OutfitChooserDialog::OnColorPartChange, this);
	feet_btn->Bind(wxEVT_BUTTON, &OutfitChooserDialog::OnColorPartChange, this);

	addon1->Bind(wxEVT_CHECKBOX, &OutfitChooserDialog::OnAddonChange, this);
	addon2->Bind(wxEVT_CHECKBOX, &OutfitChooserDialog::OnAddonChange, this);

	rand_btn->Bind(wxEVT_BUTTON, &OutfitChooserDialog::OnRandomize, this);
	fav_btn->Bind(wxEVT_BUTTON, &OutfitChooserDialog::OnAddFavorite, this);

	outfit_search->Bind(wxEVT_TEXT, &OutfitChooserDialog::OnSearchOutfit, this);

	Bind(wxEVT_BUTTON, &OutfitChooserDialog::OnOK, this, wxID_OK);

	// Update initial part selection highlight
	wxCommandEvent dummy;
	dummy.SetId(ID_COLOR_HEAD);
	OnColorPartChange(dummy);

	CenterOnParent();
}

OutfitChooserDialog::~OutfitChooserDialog() { }

void OutfitChooserDialog::OnColorPartChange(wxCommandEvent& event) {
	int id = event.GetId();
	if (id == ID_COLOR_HEAD) {
		selected_color_part = 0;
	} else if (id == ID_COLOR_BODY) {
		selected_color_part = 1;
	} else if (id == ID_COLOR_LEGS) {
		selected_color_part = 2;
	} else if (id == ID_COLOR_FEET) {
		selected_color_part = 3;
	}

	wxFont font = head_btn->GetFont();
	wxFont boldFont = font;
	boldFont.SetWeight(wxFONTWEIGHT_BOLD);
	wxFont normalFont = font;
	normalFont.SetWeight(wxFONTWEIGHT_NORMAL);

	head_btn->SetFont(selected_color_part == 0 ? boldFont : normalFont);
	body_btn->SetFont(selected_color_part == 1 ? boldFont : normalFont);
	legs_btn->SetFont(selected_color_part == 2 ? boldFont : normalFont);
	feet_btn->SetFont(selected_color_part == 3 ? boldFont : normalFont);

	Layout();
}

void OutfitChooserDialog::SelectColor(int color_id) {
	if (selected_color_part == 0) {
		current_outfit.lookHead = color_id;
	} else if (selected_color_part == 1) {
		current_outfit.lookBody = color_id;
	} else if (selected_color_part == 2) {
		current_outfit.lookLegs = color_id;
	} else if (selected_color_part == 3) {
		current_outfit.lookFeet = color_id;
	}
	UpdatePreview();
}

void OutfitChooserDialog::OnAddonChange(wxCommandEvent& event) {
	int addons = 0;
	if (addon1->GetValue()) {
		addons |= 1;
	}
	if (addon2->GetValue()) {
		addons |= 2;
	}
	current_outfit.lookAddon = addons;
	UpdatePreview();
}

void OutfitChooserDialog::OnSearchOutfit(wxCommandEvent& event) {
	selection_panel->UpdateFilter(outfit_search->GetValue());
}

void OutfitChooserDialog::UpdatePreview() {
	preview_panel->SetOutfit(current_outfit);
}

void OutfitChooserDialog::OnOK(wxCommandEvent& event) {
	current_name = name_ctrl->GetValue();
	current_speed = speed_ctrl->GetValue();

	g_preview_preferences.setName(current_name.ToUTF8().data());
	g_preview_preferences.setSpeed(current_speed);
	g_preview_preferences.setOutfit(current_outfit);
	g_preview_preferences.setFavorites(favorites);
	g_preview_preferences.save();

	EndModal(wxID_OK);
}

void OutfitChooserDialog::OnRandomize(wxCommandEvent& event) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distrib(0, TemplateOutfitLookupTableSize - 1);

	current_outfit.lookHead = distrib(gen);
	current_outfit.lookBody = distrib(gen);
	current_outfit.lookLegs = distrib(gen);
	current_outfit.lookFeet = distrib(gen);
	UpdatePreview();
}

void OutfitChooserDialog::OnAddFavorite(wxCommandEvent& event) {
	wxString label = wxGetTextFromUser("Enter a name for this favorite:", "Add to Favorites", "My Outfit");
	if (label.IsEmpty()) {
		return;
	}

	FavoriteItem item;
	item.label = label.ToUTF8().data();
	item.outfit = current_outfit;
	item.speed = speed_ctrl->GetValue();
	item.name = name_ctrl->GetValue().ToUTF8().data();

	favorites.push_back(item);
	favorites_panel->favorite_items = favorites;
	favorites_panel->UpdateVirtualSize();
}

void OutfitChooserDialog::ApplyFavorite(const FavoriteItem& item) {
	current_outfit = item.outfit;
	current_speed = item.speed;
	current_name = wxString::FromUTF8(item.name);

	speed_ctrl->SetValue(current_speed);
	name_ctrl->SetValue(current_name);

	addon1->SetValue((current_outfit.lookAddon & 1) != 0);
	addon2->SetValue((current_outfit.lookAddon & 2) != 0);

	UpdatePreview();
}

void OutfitChooserDialog::OnFavoriteRename(wxCommandEvent& event) {
	int idx = favorites_panel->selected_index;
	if (idx < 0 || idx >= (int)favorites.size()) {
		return;
	}

	wxString oldLabel = wxString::FromUTF8(favorites[idx].label);
	wxString newLabel = wxGetTextFromUser("Enter a new name for this favorite:", "Rename Favorite", oldLabel);
	if (!newLabel.IsEmpty() && newLabel != oldLabel) {
		favorites[idx].label = newLabel.ToUTF8().data();
		favorites_panel->favorite_items = favorites;
		favorites_panel->Refresh();
	}
}

void OutfitChooserDialog::OnFavoriteEdit(wxCommandEvent& event) {
	int idx = favorites_panel->selected_index;
	if (idx < 0 || idx >= (int)favorites.size()) {
		return;
	}

	if (wxMessageBox("Update this favorite with current preview settings?", "Edit Favorite", wxYES_NO | wxICON_QUESTION) == wxYES) {
		favorites[idx].outfit = current_outfit;
		favorites[idx].speed = speed_ctrl->GetValue();
		favorites[idx].name = name_ctrl->GetValue().ToUTF8().data();

		// Update cache key for the persistent icon if outfit changed
		favorites_panel->favorite_items = favorites;
		favorites_panel->Refresh();
	}
}

void OutfitChooserDialog::OnFavoriteDelete(wxCommandEvent& event) {
	int idx = favorites_panel->selected_index;
	if (idx < 0 || idx >= (int)favorites.size()) {
		return;
	}

	if (wxMessageBox("Are you sure you want to delete this favorite?", "Delete Favorite", wxYES_NO | wxICON_WARNING) == wxYES) {
		favorites.erase(favorites.begin() + idx);
		favorites_panel->favorite_items = favorites;
		favorites_panel->selected_index = -1;
		favorites_panel->UpdateVirtualSize();
		favorites_panel->Refresh();
	}
}

Outfit OutfitChooserDialog::GetOutfit() const {
	return current_outfit;
}

void OutfitChooserDialog::SetOutfit(const Outfit& outfit) {
	current_outfit = outfit;
	UpdatePreview();
}
