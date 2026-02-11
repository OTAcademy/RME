#ifndef RME_UI_TOOL_OPTIONS_SURFACE_H_
#define RME_UI_TOOL_OPTIONS_SURFACE_H_

#include "app/main.h"
#include <wx/wx.h>
#include <wx/timer.h>
#include <vector>
#include "palette/palette_common.h"

class Brush;

// A custom-drawn, high-density surface for tool options.
// Replaces the old panel-based layout with a unified, paint-optimized control.
class ToolOptionsSurface : public wxControl {
public:
	ToolOptionsSurface(wxWindow* parent);
	~ToolOptionsSurface();

	// Mandatory overrides for custom controls
	wxSize DoGetBestClientSize() const override;
	void DoSetSizeHints(int minW, int minH, int maxW, int maxH, int incW, int incH) override;

	// Event Handlers
	void OnPaint(wxPaintEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt); // No-op
	void OnMouse(wxMouseEvent& evt);
	void OnLeave(wxMouseEvent& evt);
	void OnSize(wxSizeEvent& evt);
	void OnTimer(wxTimerEvent& evt);

	// Interface
	void SetPaletteType(PaletteType type);
	void UpdateBrushSize(BrushShape shape, int size); // Called when size changes externally (e.g. shortcuts)
	void ReloadSettings();

private:
	// -- Logic --
	PaletteType current_type = TILESET_UNKNOWN;

	// Tool State
	Brush* hover_brush = nullptr;
	Brush* active_brush = nullptr;

	// Layout Constants (DIP-independent, scaled in methods)
	const int ICON_SIZE_LG = 32;
	const int ICON_SIZE_SM = 16;
	const int GRID_GAP = 4;
	const int SECTION_GAP = 12;

	const int SLIDER_LABEL_WIDTH = 70;
	const int SLIDER_TEXT_MARGIN = 40;
	const int SLIDER_VALUE_MARGIN = 8;
	const int SLIDER_THUMB_RADIUS = 5;

	const int MIN_BRUSH_SIZE = 1;
	const int MAX_BRUSH_SIZE = 15;
	const int MIN_BRUSH_THICKNESS = 1;
	const int MAX_BRUSH_THICKNESS = 100;

	// Animation
	wxTimer m_animTimer;
	float m_hoverAlpha = 0.0f;
	wxPoint m_hoverPos = wxPoint(-1, -1);

	// Helper Structures
	struct ToolRect {
		wxRect rect;
		Brush* brush;
		int id; // Command ID / Toggle ID
		wxString tooltip;
	};
	std::vector<ToolRect> tool_rects;

	// UI Elements State
	struct {
		wxRect size_slider_rect;
		wxRect thickness_slider_rect;
		wxRect preview_check_rect;
		wxRect lock_check_rect;

		bool dragging_size = false;
		bool dragging_thickness = false;
		bool hover_preview = false;
		bool hover_lock = false;
	} interactables;

	// Data Cache
	int current_size = 1;
	int current_thickness = 1;
	bool show_preview = false;
	bool lock_doors = false;

	// Internal Helpers
	void RebuildLayout();
	void DrawToolIcon(wxDC& dc, const ToolRect& tr);
	void DrawSlider(wxDC& dc, const wxRect& rect, const wxString& label, int value, int min, int max, bool active);
	void DrawCheckbox(wxDC& dc, const wxRect& rect, const wxString& label, bool value, bool hover);

	int CalculateSliderValue(const wxRect& sliderRect, int min, int max) const;

	Brush* GetBrushAt(const wxPoint& pt);
	void HandleClick(const wxPoint& pt);
	void SelectBrush(Brush* brush);
};

#endif
