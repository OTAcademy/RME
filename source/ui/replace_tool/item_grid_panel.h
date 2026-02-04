#ifndef RME_ITEM_GRID_PANEL_H_
#define RME_ITEM_GRID_PANEL_H_

#include "util/nanovg_canvas.h"
#include <vector>
#include <map>

class ItemGridPanel : public NanoVGCanvas {
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

protected:
	void OnNanoVGPaint(NVGcontext* vg, int width, int height) override;

private:
	void OnMouse(wxMouseEvent& event);
	void OnTimer(wxTimerEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMotion(wxMouseEvent& event);
	// OnMouseWheel is handled by NanoVGCanvas
	// void OnPaint is handled by NanoVGCanvas -> OnNanoVGPaint

	void UpdateVirtualSize();
	wxRect GetItemRect(int index) const;
	int HitTest(int x, int y) const;

	int GetTextureForId(uint16_t id);

	std::vector<uint16_t> allItems;
	std::vector<uint16_t> filteredItems;
	uint16_t hoveredId = 0;
	uint16_t selectedId = 0;
	Listener* listener;
	bool m_draggable = false;
	bool m_showDetails = true;
	std::map<uint16_t, wxString> m_nameOverrides;

	// Animation state
	struct HoverState {
		float alpha = 0.0f;
		float targetAlpha = 0.0f;
	};
	std::map<uint16_t, HoverState> m_hoverStates;
	wxTimer m_animTimer;

	int m_cols = 1;
	// m_scrollPos is managed by NanoVGCanvas
	int m_maxRows = 0;

	// Texture cache managed by NanoVGCanvas

	// Visual Constants (Compact styling)
	const int item_width = 96;
	const int item_height = 140;
	const int padding = 2;
};

#endif
