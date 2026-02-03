#include "ui/dialogs/outfit_selection_grid.h"
#include "ui/dialogs/outfit_chooser_dialog.h"
#include "rendering/utilities/sprite_icon_generator.h"
#include "rendering/core/graphics.h"
#include "ui/gui.h"
#include <wx/dcbuffer.h>
#include <wx/menu.h>
#include <algorithm>
#include <iterator>

namespace {
	const int OUTFIT_TILE_WIDTH = 100;
	const int OUTFIT_TILE_HEIGHT = 120;
}

OutfitSelectionGrid::OutfitSelectionGrid(wxWindow* parent, OutfitChooserDialog* owner, bool is_favorites) :
	wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxWANTS_CHARS),
	is_favorites(is_favorites),
	selected_index(-1),
	owner(owner),
	columns(1),
	item_width(OUTFIT_TILE_WIDTH),
	item_height(OUTFIT_TILE_HEIGHT),
	padding(4) {

	SetBackgroundStyle(wxBG_STYLE_PAINT);
	SetBackgroundColour(wxColour(45, 45, 45)); // Dark themed background

	Bind(wxEVT_PAINT, &OutfitSelectionGrid::OnPaint, this);
	Bind(wxEVT_SIZE, &OutfitSelectionGrid::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &OutfitSelectionGrid::OnMouse, this);
	Bind(wxEVT_ERASE_BACKGROUND, &OutfitSelectionGrid::OnEraseBackground, this);
	Bind(wxEVT_MOTION, &OutfitSelectionGrid::OnMotion, this);
	Bind(wxEVT_CONTEXT_MENU, &OutfitSelectionGrid::OnContextMenu, this);
	if (is_favorites) {
		Bind(wxEVT_MENU, &OutfitChooserDialog::OnFavoriteRename, owner, OutfitChooserDialog::ID_FAVORITE_RENAME);
		Bind(wxEVT_MENU, &OutfitChooserDialog::OnFavoriteEdit, owner, OutfitChooserDialog::ID_FAVORITE_EDIT);
		Bind(wxEVT_MENU, &OutfitChooserDialog::OnFavoriteDelete, owner, OutfitChooserDialog::ID_FAVORITE_DELETE);
	}

	SetScrollRate(0, item_height + padding);
}

void OutfitSelectionGrid::UpdateFilter(const wxString& filter) {
	if (is_favorites) {
		UpdateVirtualSize();
		return;
	}

	filtered_outfits.clear();
	std::ranges::copy_if(all_outfits, std::back_inserter(filtered_outfits), [&](const auto& item) {
		return filter.IsEmpty() || item.name.Lower().Contains(filter.Lower()) || wxString::Format("%d", item.lookType).Contains(filter);
	});

	// Try to restore selection
	auto it = std::ranges::find_if(filtered_outfits, [&](const auto& item) {
		return item.lookType == owner->GetOutfit().lookType;
	});

	if (it != filtered_outfits.end()) {
		selected_index = static_cast<int>(std::distance(filtered_outfits.begin(), it));
	} else {
		selected_index = -1;
	}

	UpdateVirtualSize();
}

void OutfitSelectionGrid::UpdateVirtualSize() {
	int width, height;
	GetClientSize(&width, &height);

	if (is_favorites) {
		columns = 4;
	} else {
		columns = std::max(1, (width - padding) / (item_width + padding));
	}

	int count = is_favorites ? favorite_items.size() : filtered_outfits.size();
	int rows = (count + columns - 1) / columns;

	SetScrollbars(0, item_height + padding, 0, rows, 0, 0);
	Refresh();
}

void OutfitSelectionGrid::OnSize(wxSizeEvent& event) {
	UpdateVirtualSize();
	event.Skip();
}

void OutfitSelectionGrid::OnEraseBackground(wxEraseEvent& event) {
	// Prevent flicker
}

void OutfitSelectionGrid::OnPaint(wxPaintEvent& event) {
	wxAutoBufferedPaintDC dc(this);
	DoPrepareDC(dc);

	dc.SetBackground(wxBrush(GetBackgroundColour()));
	dc.Clear();

	int unit_x, unit_y;
	GetViewStart(&unit_x, &unit_y);
	int view_y = unit_y * (item_height + padding);

	int cw, ch;
	GetClientSize(&cw, &ch);

	int start_row = unit_y;
	int end_row = (view_y + ch + item_height + padding - 1) / (item_height + padding);

	int count = is_favorites ? favorite_items.size() : filtered_outfits.size();
	int start_idx = start_row * columns;
	int end_idx = std::min(count, (end_row + 1) * columns);

	wxFont font = GetFont();
	font.SetPointSize(8);
	dc.SetFont(font);

	for (int i = start_idx; i < end_idx; ++i) {
		wxRect rect = GetItemRect(i);
		int lookType = is_favorites ? favorite_items[i].outfit.lookType : filtered_outfits[i].lookType;
		wxString name = is_favorites ? favorite_items[i].label : filtered_outfits[i].name;

		// Selection highlight (Professional look)
		if (i == selected_index) {
			dc.SetPen(wxPen(wxColour(200, 200, 200), 2));
			dc.SetBrush(wxBrush(wxColour(80, 80, 80)));
		} else {
			dc.SetPen(wxPen(wxColour(60, 60, 60), 1));
			dc.SetBrush(wxBrush(wxColour(50, 50, 50)));
		}
		dc.DrawRectangle(rect);

		// Draw icon
		Outfit dummy;
		if (is_favorites) {
			dummy = favorite_items[i].outfit;
		} else {
			dummy.lookType = lookType;
		}

		uint64_t cache_key = (static_cast<uint64_t>(lookType) << 32) | dummy.getColorHash();
		auto it = icon_cache.find(cache_key);
		if (it == icon_cache.end()) {
			GameSprite* spr = g_gui.gfx.getCreatureSprite(lookType);
			if (spr) {
				icon_cache[cache_key] = SpriteIconGenerator::Generate(spr, SPRITE_SIZE_64x64, dummy);
			} else {
				icon_cache[cache_key] = wxNullBitmap;
			}
			it = icon_cache.find(cache_key);
		}

		if (it->second.IsOk()) {
			int ix = rect.x + (rect.width - 64) / 2;
			int iy = rect.y + 8;
			dc.DrawBitmap(it->second, ix, iy, true);
		}

		// Draw text
		dc.SetTextForeground(*wxWHITE);
		wxString label = name;
		if (label.Length() > 14) {
			label = label.Left(12) + "..";
		}

		int tw, th;
		dc.GetTextExtent(label, &tw, &th);
		dc.DrawText(label, rect.x + (rect.width - tw) / 2, rect.y + 75);

		dc.SetTextForeground(wxColour(180, 180, 180));
		wxString idStr = wxString::Format("#%d", lookType);
		dc.GetTextExtent(idStr, &tw, &th);
		dc.DrawText(idStr, rect.x + (rect.width - tw) / 2, rect.y + 92);
	}
}

wxRect OutfitSelectionGrid::GetItemRect(int index) const {
	int row = index / columns;
	int col = index % columns;

	return wxRect(
		padding + col * (item_width + padding),
		padding + row * (item_height + padding),
		item_width,
		item_height
	);
}

int OutfitSelectionGrid::HitTest(int x, int y) const {
	int unit_x, unit_y;
	GetViewStart(&unit_x, &unit_y);

	int real_y = y + (unit_y * (item_height + padding));
	int real_x = x;

	int col = (real_x - padding) / (item_width + padding);
	int row = (real_y - padding) / (item_height + padding);

	if (col < 0 || col >= columns || row < 0) {
		return -1;
	}

	int index = row * columns + col;
	int count = is_favorites ? favorite_items.size() : filtered_outfits.size();
	if (index >= 0 && index < count) {
		if (GetItemRect(index).Contains(real_x, real_y)) {
			return index;
		}
	}
	return -1;
}

void OutfitSelectionGrid::OnMouse(wxMouseEvent& event) {
	int index = HitTest(event.GetX(), event.GetY());
	if (index != -1 && index != selected_index) {
		selected_index = index;
		if (is_favorites) {
			owner->ApplyFavorite(favorite_items[selected_index]);
		} else {
			Outfit newOutfit = owner->GetOutfit();
			newOutfit.lookType = filtered_outfits[selected_index].lookType;
			owner->SetOutfit(newOutfit);
			owner->UpdatePreview();
		}
		Refresh();
	}
}

void OutfitSelectionGrid::OnContextMenu(wxContextMenuEvent& event) {
	if (!is_favorites) {
		return;
	}

	wxPoint pt = event.GetPosition();
	if (pt == wxDefaultPosition) {
		// Keyboard-invoked context menu
		if (selected_index != -1) {
			wxRect rect = GetItemRect(selected_index);
			pt = ClientToScreen(wxPoint(rect.x + rect.width / 2, rect.y + rect.height / 2));
		} else {
			return;
		}
	} else {
		pt = ScreenToClient(pt);
		int index = HitTest(pt.x, pt.y);
		if (index != -1) {
			selected_index = index;
			owner->ApplyFavorite(favorite_items[selected_index]);
			Refresh();
		} else {
			return;
		}
	}

	wxMenu menu;
	menu.Append(OutfitChooserDialog::ID_FAVORITE_RENAME, "Rename...");
	menu.Append(OutfitChooserDialog::ID_FAVORITE_EDIT, "Update with Current");
	menu.Append(OutfitChooserDialog::ID_FAVORITE_DELETE, "Delete");

	PopupMenu(&menu);
}

void OutfitSelectionGrid::OnMotion(wxMouseEvent& event) {
	int index = HitTest(event.GetX(), event.GetY());
	if (index != -1) {
		SetCursor(wxCursor(wxCURSOR_HAND));
	} else {
		SetCursor(wxNullCursor);
	}
	event.Skip();
}
