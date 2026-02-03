#ifndef RME_ITEM_GRID_PANEL_H_
#define RME_ITEM_GRID_PANEL_H_

#include "app/main.h"
#include <wx/glcanvas.h>
#include <wx/timer.h>
#include <nanovg.h>
#include <vector>
#include <map>

class ItemGridPanel : public wxGLCanvas {
public:
	class Listener {
	public:
		virtual ~Listener() { }
		virtual void OnItemSelected(ItemGridPanel* source, uint16_t itemId) = 0;
	};

	ItemGridPanel(wxWindow* parent, Listener* listener);
	virtual ~ItemGridPanel();

	void SetDraggable(bool check);
	void SetItems(const std::vector<uint16_t>& items);
	void SetFilter(const wxString& filter);
	void SetOverrideNames(const std::map<uint16_t, wxString>& names);
	void SetShowDetails(bool show);

	// wxWindow overrides
	wxSize DoGetBestClientSize() const override;

private:
	void OnPaint(wxPaintEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMouseWheel(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);

	void UpdateVirtualSize();
	wxRect GetItemRect(int index) const;
	int HitTest(int x, int y) const;

	void InitGL();
	int GetTextureForId(uint16_t id);
	void ClearTextureCache();

	std::vector<uint16_t> allItems;
	std::vector<uint16_t> filteredItems;
	uint16_t hoveredId = 0;
	uint16_t selectedId = 0;
	Listener* listener;
	bool m_draggable = false;
	bool m_showDetails = true;
	std::map<uint16_t, wxString> m_nameOverrides;

	// Animation state (retaining this for now, though standard OutfitGrid doesn't emphasize it as much, it's a nice touch)
	struct HoverState {
		float alpha = 0.0f;
		float targetAlpha = 0.0f;
	};
	std::map<uint16_t, HoverState> m_hoverStates;
	wxTimer m_animTimer;

	int m_cols = 1;
	int m_scrollPos = 0; // Scroll position in units (rows * (item_height + padding))
	int m_maxRows = 0;

	wxGLContext* m_glContext = nullptr;
	NVGcontext* m_nvg = nullptr;
	std::map<uint16_t, int> m_textureCache; // ItemId -> NanoVG Image Index

	// Visual Constants (Compact styling)
	// Need slightly more height for 3 lines of text (Name, SID, CID) if we want them distinct
	// 64px sprite + 14px name + 12px SID + 12px CID + padding
	const int item_width = 96;
	const int item_height = 140;
	const int padding = 2;
};

#endif
