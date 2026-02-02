#ifndef RME_PALETTE_CONTROLS_VIRTUAL_BRUSH_GRID_H_
#define RME_PALETTE_CONTROLS_VIRTUAL_BRUSH_GRID_H_

#include "app/main.h"
#include "palette/panels/brush_panel.h"
#include "ui/dcbutton.h" // For RenderSize

class VirtualBrushGrid : public wxScrolledWindow, public BrushBoxInterface {
public:
	VirtualBrushGrid(wxWindow* parent, const TilesetCategory* _tileset, RenderSize rsz);
	~VirtualBrushGrid();

	wxWindow* GetSelfWindow() override {
		return this;
	}

	// BrushBoxInterface
	void SelectFirstBrush() override;
	Brush* GetSelectedBrush() const override;
	bool SelectBrush(const Brush* brush) override;

	// Event Handlers
	void OnPaint(wxPaintEvent& event);
	void OnSize(wxSizeEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnEraseBackground(wxEraseEvent& event);

protected:
	void OnMotion(wxMouseEvent& event);
	void UpdateVirtualSize();
	int HitTest(int x, int y) const;
	wxRect GetItemRect(int index) const;

	RenderSize icon_size;
	int selected_index;
	int columns;
	int item_size;
	int padding;
	bool is_dragging;
};

#endif
