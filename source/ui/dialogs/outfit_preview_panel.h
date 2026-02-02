#ifndef RME_UI_DIALOGS_OUTFIT_PREVIEW_PANEL_H_
#define RME_UI_DIALOGS_OUTFIT_PREVIEW_PANEL_H_

#include "app/main.h"
#include "game/outfit.h"
#include <wx/panel.h>

class OutfitPreviewPanel : public wxPanel {
public:
	OutfitPreviewPanel(wxWindow* parent, const Outfit& outfit);
	void SetOutfit(const Outfit& outfit);
	void OnPaint(wxPaintEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnWheel(wxMouseEvent& event);

private:
	Outfit preview_outfit;
	int preview_direction; // 0, 1, 2, 3 (South, East, North, West)
};

#endif
