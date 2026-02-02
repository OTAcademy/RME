#ifndef RME_UI_DIALOGS_OUTFIT_CHOOSER_DIALOG_H_
#define RME_UI_DIALOGS_OUTFIT_CHOOSER_DIALOG_H_

#include "app/main.h"
#include "game/outfit.h"
#include <wx/srchctrl.h>
#include <map>
#include <vector>

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
		ID_OUTFIT_START = 31000,
		ID_FAVORITE_START = 32000
	};

	OutfitChooserDialog(wxWindow* parent, const Outfit& current_outfit);
	virtual ~OutfitChooserDialog();

	Outfit GetOutfit() const;
	wxString GetName() const {
		return current_name;
	}
	int GetSpeed() const {
		return current_speed;
	}

private:
	class PreviewPanel : public wxPanel {
	public:
		PreviewPanel(wxWindow* parent, const Outfit& outfit);
		void SetOutfit(const Outfit& outfit);
		void OnPaint(wxPaintEvent& event);
		void OnMouse(wxMouseEvent& event);
		void OnWheel(wxMouseEvent& event);

	private:
		Outfit preview_outfit;
		int preview_direction; // 0, 1, 2, 3 (South, East, North, West)
	};

	struct OutfitItem {
		int lookType;
		wxString name;
	};

	struct FavoriteItem {
		Outfit outfit;
		wxString name; // customName in cpp
		int speed;
		wxString label; // entryLabel in cpp/header
	};

	class OutfitSelectionGrid : public wxScrolledWindow {
	public:
		OutfitSelectionGrid(wxWindow* parent, OutfitChooserDialog* owner, bool is_favorites = false);
		void UpdateFilter(const wxString& filter);
		void UpdateVirtualSize();
		std::vector<OutfitItem> all_outfits;
		std::vector<OutfitItem> filtered_outfits;
		std::vector<FavoriteItem> favorite_items;
		bool is_favorites;

	private:
		void OnPaint(wxPaintEvent& event);
		void OnSize(wxSizeEvent& event);
		void OnMouse(wxMouseEvent& event);
		void OnEraseBackground(wxEraseEvent& event);
		void OnMotion(wxMouseEvent& event);

		int HitTest(int x, int y) const;
		wxRect GetItemRect(int index) const;

		OutfitChooserDialog* owner;
		std::map<int, wxBitmap> icon_cache;
		int selected_index;
		int columns;
		int item_width;
		int item_height;
		int padding;
	};

	void OnColorClick(wxCommandEvent& event);
	void OnColorPartChange(wxCommandEvent& event);
	void OnAddonChange(wxCommandEvent& event);
	void OnSearchOutfit(wxCommandEvent& event);
	void OnRandomize(wxCommandEvent& event);
	void OnAddFavorite(wxCommandEvent& event);
	void OnOK(wxCommandEvent& event);

	void UpdatePreview();
	void SelectColor(int color_id);
	void LoadPreferences();
	void SavePreferences();
	void ApplyFavorite(const FavoriteItem& fav);

	Outfit current_outfit;
	int current_speed;
	wxString current_name;
	std::vector<FavoriteItem> favorites;
	int selected_color_part; // 0: head, 1: primary, 2: secondary, 3: detail

	PreviewPanel* preview_panel;
	wxSearchCtrl* outfit_search;
	OutfitSelectionGrid* selection_panel;
	OutfitSelectionGrid* favorites_panel;

	wxCheckBox* addon1;
	wxCheckBox* addon2;

	wxSpinCtrl* speed_ctrl;
	wxTextCtrl* name_ctrl;

	wxButton* head_btn;
	wxButton* body_btn;
	wxButton* legs_btn;
	wxButton* feet_btn;

	std::vector<wxButton*> color_buttons;
};

#endif
