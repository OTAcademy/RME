#ifndef RME_UI_DIALOGS_OUTFIT_CHOOSER_DIALOG_H_
#define RME_UI_DIALOGS_OUTFIT_CHOOSER_DIALOG_H_

#include "app/main.h"
#include "game/outfit.h"
#include "game/preview_preferences.h"
#include <wx/srchctrl.h>
#include <map>
#include <vector>

#include "ui/dialogs/outfit_preview_panel.h"
#include "ui/dialogs/outfit_selection_grid.h"

class OutfitChooserDialog : public wxDialog {
public:
	enum {
		ID_SEARCH = 30000,
		ID_ADDON1,
		ID_ADDON2,
		ID_COLOR_HEAD,
		ID_COLOR_BODY,
		ID_COLOR_LEGS,
		ID_COLOR_FEET,
		ID_COLOR_START,
		ID_SPEED,
		ID_NAME,
		ID_RANDOMIZE,
		ID_ADD_FAVORITE,
		ID_FAVORITE_RENAME,
		ID_FAVORITE_EDIT,
		ID_FAVORITE_DELETE,
		ID_OUTFIT_START = 31000,
		ID_FAVORITE_START = 32000
	};

	OutfitChooserDialog(wxWindow* parent, const Outfit& current_outfit);
	virtual ~OutfitChooserDialog();

	Outfit GetOutfit() const;
	void SetOutfit(const Outfit& outfit);

	wxString GetName() const {
		return current_name;
	}
	int GetSpeed() const {
		return current_speed;
	}

	void UpdatePreview();
	void ApplyFavorite(const FavoriteItem& fav);

	// Public for OutfitSelectionGrid access
	void OnFavoriteRename(wxCommandEvent& event);
	void OnFavoriteEdit(wxCommandEvent& event);
	void OnFavoriteDelete(wxCommandEvent& event);

	Outfit current_outfit; // Made public or keep friend? Friend is messy with circular includes. Let's make it public for now or add getter/setter.
	// Actually, OutfitSelectionGrid accesses current_outfit. Let's use getter/setter in the grid.
	// But current_outfit is used extensively. Let's keep it private and add friends or accessors.
	// Wait, I already added GetOutfit/SetOutfit. I'll make members public/accessors as needed.
	// For now, I'll keep the private section but moving specific handlers to public so Grid can bind them.

private:
	void OnColorPartChange(wxCommandEvent& event);
	void OnAddonChange(wxCommandEvent& event);
	void OnSearchOutfit(wxCommandEvent& event);
	void OnRandomize(wxCommandEvent& event);
	void OnAddFavorite(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	void UpdateColorSelection();
	void SelectColor(int color_id);

	int current_speed;
	wxString current_name;
	std::vector<FavoriteItem> favorites;
	int selected_color_part; // 0: head, 1: primary, 2: secondary, 3: detail

	OutfitPreviewPanel* preview_panel;
	wxSearchCtrl* outfit_search;
	OutfitSelectionGrid* selection_panel;
	OutfitSelectionGrid* favorites_panel;

	wxCheckBox* addon1;
	wxCheckBox* addon2;
	wxCheckBox* colorable_only_check;

	wxSpinCtrl* speed_ctrl;
	wxTextCtrl* name_ctrl;

	wxButton* head_btn;
	wxButton* body_btn;
	wxButton* legs_btn;
	wxButton* feet_btn;

	std::vector<wxWindow*> color_buttons;
};

#endif
