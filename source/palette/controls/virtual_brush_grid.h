#ifndef RME_PALETTE_CONTROLS_VIRTUAL_BRUSH_GRID_H_
#define RME_PALETTE_CONTROLS_VIRTUAL_BRUSH_GRID_H_

#include "app/main.h"
#include "util/nanovg_canvas.h"
#include "palette/panels/brush_panel.h"
#include "ui/dcbutton.h" // For RenderSize

/**
 * @class VirtualBrushGrid
 * @brief High-performance brush grid using NanoVG for GPU-accelerated rendering.
 *
 * This control displays a grid of brush icons with virtual scrolling,
 * supporting thousands of brushes at 60fps. Uses texture caching for
 * efficient sprite rendering.
 */
class VirtualBrushGrid : public NanoVGCanvas, public BrushBoxInterface {
public:
	/**
	 * @brief Constructs a VirtualBrushGrid.
	 * @param parent Parent window
	 * @param _tileset The tileset category containing brushes
	 * @param rsz Icon render size (16x16 or 32x32)
	 */
	VirtualBrushGrid(wxWindow* parent, const TilesetCategory* _tileset, RenderSize rsz);
	~VirtualBrushGrid() override;

	wxWindow* GetSelfWindow() override {
		return this;
	}

	// BrushBoxInterface
	void SelectFirstBrush() override;
	Brush* GetSelectedBrush() const override;
	bool SelectBrush(const Brush* brush) override;

protected:
	/**
	 * @brief Performs NanoVG rendering of the brush grid.
	 * @param vg NanoVG context
	 * @param width Canvas width
	 * @param height Canvas height
	 */
	void OnNanoVGPaint(NVGcontext* vg, int width, int height) override;

	wxSize DoGetBestClientSize() const override;

	// Event Handlers
	void OnMouseDown(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);

	// Internal helpers
	void UpdateLayout();
	int HitTest(int x, int y) const;
	wxRect GetItemRect(int index) const;
	int GetOrCreateBrushTexture(NVGcontext* vg, Brush* brush);
	void DrawBrushItem(NVGcontext* vg, int index, const wxRect& rect);

	RenderSize icon_size;
	int selected_index;
	int hover_index;
	int columns;
	int item_size;
	int padding;

	// Animation state
	wxTimer* m_animTimer;
	void OnTimer(wxTimerEvent& event);
};

#endif
