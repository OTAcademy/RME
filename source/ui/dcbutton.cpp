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

#include "app/settings.h"

#include "ui/dcbutton.h"
#include "game/sprites.h"
#include "ui/gui.h"

#include <glad/glad.h>
#include <nanovg.h>

IMPLEMENT_DYNAMIC_CLASS(DCButton, NanoVGCanvas)

DCButton::DCButton() :
	NanoVGCanvas(nullptr, wxID_ANY, 0),
	type(DC_BTN_NORMAL),
	state(false),
	size(RENDER_SIZE_16x16),
	sprite(nullptr),
	overlay(nullptr) {
	SetSize(36, 36);
	SetMinSize(wxSize(36, 36));
	Bind(wxEVT_LEFT_DOWN, &DCButton::OnClick, this);
	SetSprite(0);
}

DCButton::DCButton(wxWindow* parent, wxWindowID id, wxPoint pos, int type, RenderSize sz, int sprite_id) :
	NanoVGCanvas(parent, id, 0),
	type(type),
	state(false),
	size(sz),
	sprite(nullptr),
	overlay(nullptr) {

	wxSize winSize;
	if (sz == RENDER_SIZE_64x64) {
		winSize = wxSize(68, 68);
	} else if (sz == RENDER_SIZE_32x32) {
		winSize = wxSize(36, 36);
	} else {
		winSize = wxSize(20, 20);
	}

	SetSize(winSize);
	SetMinSize(winSize);
	if (pos != wxDefaultPosition) {
		SetPosition(pos);
	}

	Bind(wxEVT_LEFT_DOWN, &DCButton::OnClick, this);
	SetSprite(sprite_id);
}

DCButton::~DCButton() {
	////
}

void DCButton::SetSprite(int _sprid) {
	if (_sprid != 0) {
		sprite = g_gui.gfx.getSprite(_sprid);
	} else {
		sprite = nullptr;
	}
	Refresh();
}

void DCButton::SetSprite(Sprite* _sprite) {
	sprite = _sprite;
	Refresh();
}

void DCButton::SetOverlay(Sprite* espr) {
	overlay = espr;
	Refresh();
}

void DCButton::SetValue(bool val) {
	ASSERT(type == DC_BTN_TOGGLE);
	bool oldval = val;
	state = val;
	if (state == oldval) {
		// Cheap to change value to the old one (which is done ALOT)
		if (GetValue() && g_settings.getInteger(Config::USE_GUI_SELECTION_SHADOW)) {
			SetOverlay(g_gui.gfx.getSprite(EDITOR_SPRITE_SELECTION_MARKER));
		} else {
			SetOverlay(nullptr);
		}
		Refresh();
	}
}

bool DCButton::GetValue() const {
	ASSERT(type == DC_BTN_TOGGLE);
	return state;
}

wxSize DCButton::DoGetBestClientSize() const {
	if (size == RENDER_SIZE_64x64) {
		return FromDIP(wxSize(68, 68));
	}
	if (size == RENDER_SIZE_32x32) {
		return FromDIP(wxSize(36, 36));
	}
	return FromDIP(wxSize(20, 20));
}

void DCButton::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	if (g_gui.gfx.isUnloaded()) {
		return;
	}

	int size_x = 20, size_y = 20;

	if (size == RENDER_SIZE_16x16) {
		size_x = 20;
		size_y = 20;
	} else if (size == RENDER_SIZE_32x32) {
		size_x = 36;
		size_y = 36;
	} else if (size == RENDER_SIZE_64x64) {
		size_x = 68;
		size_y = 68;
	}

	// Background
	nvgBeginPath(vg);
	nvgRect(vg, 0, 0, size_x, size_y);
	nvgFillColor(vg, nvgRGBA(0, 0, 0, 255));
	nvgFill(vg);

	if (type == DC_BTN_TOGGLE && GetValue()) {
		DrawSunkenBorder(vg, static_cast<float>(size_x), static_cast<float>(size_y));
	} else {
		DrawRaisedBorder(vg, static_cast<float>(size_x), static_cast<float>(size_y));
	}

	if (sprite) {
		int tex = GetOrCreateSpriteTexture(vg, sprite);
		if (tex > 0) {
			int imgSize = 32;
			if (size == RENDER_SIZE_16x16) {
				imgSize = 16;
			} else if (size == RENDER_SIZE_32x32) {
				imgSize = 32;
			} else if (size == RENDER_SIZE_64x64) {
				imgSize = 64; // Not supported in original?
			}

			NVGpaint imgPaint = nvgImagePattern(vg, 2, 2, imgSize, imgSize, 0, tex, 1.0f);
			nvgBeginPath(vg);
			nvgRect(vg, 2, 2, imgSize, imgSize);
			nvgFillPaint(vg, imgPaint);
			nvgFill(vg);

			if (overlay && type == DC_BTN_TOGGLE && GetValue()) {
				int overlayTex = GetOrCreateSpriteTexture(vg, overlay);
				if (overlayTex > 0) {
					NVGpaint ovPaint = nvgImagePattern(vg, 2, 2, imgSize, imgSize, 0, overlayTex, 1.0f);
					nvgBeginPath(vg);
					nvgRect(vg, 2, 2, imgSize, imgSize);
					nvgFillPaint(vg, ovPaint);
					nvgFill(vg);
				}
			}
		}
	}
}

void DCButton::OnClick(wxMouseEvent& WXUNUSED(evt)) {
	wxCommandEvent event(type == DC_BTN_TOGGLE ? wxEVT_COMMAND_TOGGLEBUTTON_CLICKED : wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	event.SetEventObject(this);

	if (type == DC_BTN_TOGGLE) {
		SetValue(!GetValue());
	}
	SetFocus();

	GetEventHandler()->ProcessEvent(event);
}

void DCButton::DrawSunkenBorder(NVGcontext* vg, float size_x, float size_y) {
	NVGcolor dark_highlight = nvgRGBA(212, 208, 200, 255);
	NVGcolor light_shadow = nvgRGBA(128, 128, 128, 255);
	NVGcolor highlight = nvgRGBA(255, 255, 255, 255);
	NVGcolor shadow = nvgRGBA(64, 64, 64, 255);

	nvgStrokeWidth(vg, 1.0f);

	nvgBeginPath(vg);
	nvgMoveTo(vg, 0.5f, size_y - 0.5f);
	nvgLineTo(vg, 0.5f, 0.5f);
	nvgLineTo(vg, size_x - 0.5f, 0.5f);
	nvgStrokeColor(vg, shadow);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, 1.5f, size_y - 1.5f);
	nvgLineTo(vg, 1.5f, 1.5f);
	nvgLineTo(vg, size_x - 1.5f, 1.5f);
	nvgStrokeColor(vg, light_shadow);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, size_x - 1.5f, 1.5f);
	nvgLineTo(vg, size_x - 1.5f, size_y - 1.5f);
	nvgLineTo(vg, 1.5f, size_y - 1.5f);
	nvgStrokeColor(vg, dark_highlight);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, size_x - 0.5f, 0.5f);
	nvgLineTo(vg, size_x - 0.5f, size_y - 0.5f);
	nvgLineTo(vg, 0.5f, size_y - 0.5f);
	nvgStrokeColor(vg, highlight);
	nvgStroke(vg);
}

void DCButton::DrawRaisedBorder(NVGcontext* vg, float size_x, float size_y) {
	NVGcolor dark_highlight = nvgRGBA(212, 208, 200, 255);
	NVGcolor light_shadow = nvgRGBA(128, 128, 128, 255);
	NVGcolor highlight = nvgRGBA(255, 255, 255, 255);
	NVGcolor shadow = nvgRGBA(64, 64, 64, 255);

	nvgStrokeWidth(vg, 1.0f);

	nvgBeginPath(vg);
	nvgMoveTo(vg, 0.5f, size_y - 0.5f);
	nvgLineTo(vg, 0.5f, 0.5f);
	nvgLineTo(vg, size_x - 0.5f, 0.5f);
	nvgStrokeColor(vg, highlight);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, 1.5f, size_y - 1.5f);
	nvgLineTo(vg, 1.5f, 1.5f);
	nvgLineTo(vg, size_x - 1.5f, 1.5f);
	nvgStrokeColor(vg, dark_highlight);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, size_x - 1.5f, 1.5f);
	nvgLineTo(vg, size_x - 1.5f, size_y - 1.5f);
	nvgLineTo(vg, 1.5f, size_y - 1.5f);
	nvgStrokeColor(vg, light_shadow);
	nvgStroke(vg);

	nvgBeginPath(vg);
	nvgMoveTo(vg, size_x - 0.5f, 0.5f);
	nvgLineTo(vg, size_x - 0.5f, size_y - 0.5f);
	nvgLineTo(vg, 0.5f, size_y - 0.5f);
	nvgStrokeColor(vg, shadow);
	nvgStroke(vg);
}
