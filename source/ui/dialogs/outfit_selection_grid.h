#ifndef RME_UI_DIALOGS_OUTFIT_SELECTION_GRID_H_
#define RME_UI_DIALOGS_OUTFIT_SELECTION_GRID_H_

#include "app/main.h"
#include "game/outfit.h"
#include "game/preview_preferences.h"
#include <wx/scrolwin.h>
#include <vector>
#include <map>

class OutfitChooserDialog; // Forward declaration

struct OutfitItem {
	int lookType;
	wxString name;
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
	int selected_index;

private:
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnMotion(wxMouseEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);

	int HitTest(int x, int y) const;
	wxRect GetItemRect(int index) const;

	OutfitChooserDialog* owner;
	std::map<uint64_t, wxBitmap> icon_cache;
	int columns;
	int item_width;
	int item_height;
	int padding;
};

#endif
