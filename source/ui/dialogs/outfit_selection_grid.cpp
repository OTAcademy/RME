#include "ui/dialogs/outfit_selection_grid.h"
#include "ui/dialogs/outfit_chooser_dialog.h"
#include "rendering/utilities/sprite_icon_generator.h"
#include "rendering/core/graphics.h"
#include "ui/gui.h"

#include <glad/glad.h>
#define NANOVG_GL3
#include <nanovg.h>
#include <nanovg_gl.h>

#include <algorithm>
#include <iterator>

namespace {
	const int OUTFIT_TILE_WIDTH = 100;
	const int OUTFIT_TILE_HEIGHT = 120;
}

OutfitSelectionGrid::OutfitSelectionGrid(wxWindow* parent, OutfitChooserDialog* owner, bool is_favorites) :
	NanoVGCanvas(parent, wxID_ANY, wxVSCROLL | wxWANTS_CHARS),
	is_favorites(is_favorites),
	selected_index(-1),
	owner(owner),
	columns(1),
	item_width(OUTFIT_TILE_WIDTH),
	item_height(OUTFIT_TILE_HEIGHT),
	padding(4),
	hover_index(-1) {

	Bind(wxEVT_SIZE, &OutfitSelectionGrid::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &OutfitSelectionGrid::OnMouse, this);
	Bind(wxEVT_MOTION, &OutfitSelectionGrid::OnMotion, this);
	Bind(wxEVT_CONTEXT_MENU, &OutfitSelectionGrid::OnContextMenu, this);
	if (is_favorites) {
		Bind(wxEVT_MENU, &OutfitChooserDialog::OnFavoriteRename, owner, OutfitChooserDialog::ID_FAVORITE_RENAME);
		Bind(wxEVT_MENU, &OutfitChooserDialog::OnFavoriteEdit, owner, OutfitChooserDialog::ID_FAVORITE_EDIT);
		Bind(wxEVT_MENU, &OutfitChooserDialog::OnFavoriteDelete, owner, OutfitChooserDialog::ID_FAVORITE_DELETE);
	}
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
	int width = GetClientSize().x;

	if (is_favorites) {
		columns = 4;
	} else {
		columns = std::max(1, (width - padding) / (item_width + padding));
	}

	int count = is_favorites ? favorite_items.size() : filtered_outfits.size();
	int rows = (count + columns - 1) / columns;
	int contentHeight = rows * (item_height + padding) + padding;

	UpdateScrollbar(contentHeight);
	Refresh();
}

void OutfitSelectionGrid::OnSize(wxSizeEvent& event) {
	UpdateVirtualSize();
	event.Skip();
}

wxSize OutfitSelectionGrid::DoGetBestClientSize() const {
	return wxSize(430, 300); // Reasonable default
}

int OutfitSelectionGrid::GetOrCreateOutfitImage(NVGcontext* vg, int lookType, const Outfit& outfit) {
	uint32_t cache_key = (static_cast<uint32_t>(lookType) ^ static_cast<uint32_t>(outfit.getColorHash()));

	int existing = GetCachedImage(cache_key);
	if (existing > 0) {
		return existing;
	}

	GameSprite* spr = g_gui.gfx.getCreatureSprite(lookType);
	if (!spr) {
		return 0;
	}

	wxBitmap bmp = SpriteIconGenerator::Generate(spr, SPRITE_SIZE_64x64, outfit);
	wxImage img = bmp.ConvertToImage();

	if (!img.IsOk()) {
		return 0;
	}

	// Convert to RGBA buffer for NanoVG
	int w = img.GetWidth();
	int h = img.GetHeight();
	std::vector<uint8_t> rgba(w * h * 4);

	const uint8_t* data = img.GetData();
	const uint8_t* alpha = img.GetAlpha();

	for (int i = 0; i < w * h; ++i) {
		rgba[i * 4 + 0] = data[i * 3 + 0];
		rgba[i * 4 + 1] = data[i * 3 + 1];
		rgba[i * 4 + 2] = data[i * 3 + 2];
		if (alpha) {
			rgba[i * 4 + 3] = alpha[i];
		} else {
			rgba[i * 4 + 3] = 255;
		}
	}

	return GetOrCreateImage(cache_key, rgba.data(), w, h);
}

void OutfitSelectionGrid::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	if (width <= 0) return;

	int scrollPos = GetScrollPosition();

	// Draw background
	nvgBeginPath(vg);
	nvgRect(vg, 0, 0, width, height); // Note: height here is canvas height, not content height. But scroll is handled by transform.
	// Actually, width and height passed to OnNanoVGPaint are client size.
	// Coordinate system is shifted by -scrollPos.
	// So to fill the visible background, we need to draw at (0, scrollPos, width, height).
	// NanoVGCanvas implementation:
	// nvgTranslate(m_nvg, 0, -m_scrollPos);
	// OnNanoVGPaint(m_nvg, width, height);
	// So (0,0) is at top of virtual content.
	// We want to fill the visible area background.
	// Visible area is from y=scrollPos to y=scrollPos+height.

	nvgBeginPath(vg);
	nvgRect(vg, 0, scrollPos, width, height);
	nvgFillColor(vg, nvgRGBA(45, 45, 45, 255));
	nvgFill(vg);

	int start_row = scrollPos / (item_height + padding);
	int end_row = (scrollPos + height + (item_height + padding) - 1) / (item_height + padding);

	int count = is_favorites ? favorite_items.size() : filtered_outfits.size();
	int start_idx = start_row * columns;
	int end_idx = std::min(count, (end_row + 1) * columns);

	for (int i = start_idx; i < end_idx; ++i) {
		wxRect rect = GetItemRect(i);

		int lookType = is_favorites ? favorite_items[i].outfit.lookType : filtered_outfits[i].lookType;
		wxString name = is_favorites ? favorite_items[i].label : filtered_outfits[i].name;

		// Card background
		nvgBeginPath(vg);
		nvgRoundedRect(vg, rect.x, rect.y, rect.width, rect.height, 4.0f);

		if (i == selected_index) {
			nvgFillColor(vg, nvgRGBA(80, 80, 80, 255));
		} else if (i == hover_index) {
			nvgFillColor(vg, nvgRGBA(60, 60, 60, 255));
		} else {
			nvgFillColor(vg, nvgRGBA(50, 50, 50, 255));
		}
		nvgFill(vg);

		// Border
		nvgBeginPath(vg);
		nvgRoundedRect(vg, rect.x + 0.5f, rect.y + 0.5f, rect.width - 1.0f, rect.height - 1.0f, 4.0f);
		if (i == selected_index) {
			nvgStrokeColor(vg, nvgRGBA(200, 200, 200, 255));
			nvgStrokeWidth(vg, 2.0f);
		} else {
			nvgStrokeColor(vg, nvgRGBA(60, 60, 60, 255));
			nvgStrokeWidth(vg, 1.0f);
		}
		nvgStroke(vg);

		// Icon
		Outfit outfit;
		if (is_favorites) {
			outfit = favorite_items[i].outfit;
		} else {
			outfit.lookType = lookType;
		}

		int imgId = GetOrCreateOutfitImage(vg, lookType, outfit);
		if (imgId > 0) {
			int ix = rect.x + (rect.width - 64) / 2;
			int iy = rect.y + 8;

			NVGpaint imgPaint = nvgImagePattern(vg, ix, iy, 64, 64, 0, imgId, 1.0f);
			nvgBeginPath(vg);
			nvgRect(vg, ix, iy, 64, 64);
			nvgFillPaint(vg, imgPaint);
			nvgFill(vg);
		}

		// Text
		nvgFontSize(vg, 12.0f);
		nvgFontFace(vg, "sans");
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);

		// Name
		nvgFillColor(vg, nvgRGBA(255, 255, 255, 255));
		if (name.length() > 14) {
			name = name.Mid(0, 12) + "..";
		}
		nvgText(vg, rect.x + rect.width / 2, rect.y + 75, name.ToUTF8().data(), nullptr);

		// ID
		wxString idStr = wxString::Format("#%d", lookType);
		nvgFillColor(vg, nvgRGBA(180, 180, 180, 255));
		nvgText(vg, rect.x + rect.width / 2, rect.y + 92, idStr.ToUTF8().data(), nullptr);
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
	int scrollPos = GetScrollPosition();
	int real_y = y + scrollPos;
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
			int scrollPos = GetScrollPosition();
			int cx = rect.x + rect.width / 2;
			int cy = rect.y + rect.height / 2 - scrollPos;
			pt = ClientToScreen(wxPoint(cx, cy));
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
	if (index != hover_index) {
		hover_index = index;
		Refresh();
	}

	if (index != -1) {
		SetCursor(wxCursor(wxCURSOR_HAND));
	} else {
		SetCursor(wxNullCursor);
	}
	event.Skip();
}
