#ifndef RME_INGAME_PREVIEW_WINDOW_H_
#define RME_INGAME_PREVIEW_WINDOW_H_

#include "app/main.h"
#include <memory>

class Editor;

namespace IngamePreview {

	class IngamePreviewCanvas;

	class IngamePreviewWindow : public wxPanel {
	public:
		IngamePreviewWindow(wxWindow* parent, Editor& editor);
		~IngamePreviewWindow();

		void OnUpdateTimer(wxTimerEvent& event);
		void OnToggleFollow(wxCommandEvent& event);
		void OnToggleLighting(wxCommandEvent& event);
		void OnAmbientSlider(wxCommandEvent& event);
		void OnIntensitySlider(wxCommandEvent& event);
		void OnViewportWidthUp(wxCommandEvent& event);
		void OnViewportWidthDown(wxCommandEvent& event);
		void OnViewportHeightUp(wxCommandEvent& event);
		void OnViewportHeightDown(wxCommandEvent& event);

		void UpdateState();

	private:
		Editor& editor;
		IngamePreviewCanvas* canvas;
		wxTimer update_timer;

		// UI Controls
		wxBitmapToggleButton* follow_btn;
		wxBitmapToggleButton* lighting_btn;
		wxSlider* ambient_slider;
		wxSlider* intensity_slider;

		wxTextCtrl* viewport_x_text;
		wxTextCtrl* viewport_y_text;
		wxButton* viewport_w_up;
		wxButton* viewport_w_down;
		wxButton* viewport_h_up;
		wxButton* viewport_h_down;

		bool follow_selection;

		DECLARE_EVENT_TABLE()
	};

} // namespace IngamePreview

#endif // RME_INGAME_PREVIEW_WINDOW_H_
