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

#ifndef RME_DC_BUTTON_H
#define RME_DC_BUTTON_H

#include "util/nanovg_canvas.h"

class Sprite;
class GameSprite;
class EditorSprite;

enum {
	DC_BTN_NORMAL,
	DC_BTN_TOGGLE,
};

enum RenderSize {
	RENDER_SIZE_16x16,
	RENDER_SIZE_32x32,
	RENDER_SIZE_64x64,
};

class DCButton : public NanoVGCanvas {
public:
	DCButton();
	DCButton(wxWindow* parent, wxWindowID id, wxPoint pos, int type, RenderSize sz, int sprite_id);
	virtual ~DCButton();

	void SetValue(bool val);
	bool GetValue() const;

	void SetSprite(int id);
	void SetSprite(Sprite* sprite);

	void OnClick(wxMouseEvent&);

protected:
	void OnNanoVGPaint(NVGcontext* vg, int width, int height) override;
	wxSize DoGetBestClientSize() const override;

	void DrawSunkenBorder(NVGcontext* vg, float x, float y);
	void DrawRaisedBorder(NVGcontext* vg, float x, float y);

	void SetOverlay(Sprite* espr);

	int type;
	bool state; // pressed/unpressed
	RenderSize size;
	Sprite* sprite;
	Sprite* overlay;

	DECLARE_DYNAMIC_CLASS(DCButton)
};

#endif
