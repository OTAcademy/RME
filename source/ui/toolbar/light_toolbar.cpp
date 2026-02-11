//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/light_toolbar.h"
#include "ui/gui.h"
#include "ui/gui_ids.h"
#include "app/settings.h"
#include "util/image_manager.h"

const wxString LightToolBar::PANE_NAME = "light_toolbar";

LightToolBar::LightToolBar(wxWindow* parent) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));

	toolbar = newd wxAuiToolBar(parent, TOOLBAR_LIGHT, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORZ_TEXT);
	toolbar->SetToolBitmapSize(icon_size);

	wxStaticText* intensity_label = newd wxStaticText(toolbar, wxID_ANY, "Intensity:");
	light_slider = newd wxSlider(toolbar, ID_LIGHT_INTENSITY_SLIDER, 100, 0, 200, wxDefaultPosition, parent->FromDIP(wxSize(100, 20)));
	light_slider->SetToolTip("Global Light Intensity");

	wxStaticText* ambient_label = newd wxStaticText(toolbar, wxID_ANY, "Ambient:");
	ambient_slider = newd wxSlider(toolbar, ID_AMBIENT_LIGHT_SLIDER, 50, 0, 100, wxDefaultPosition, parent->FromDIP(wxSize(100, 20)));
	ambient_slider->SetToolTip("Ambient Light Level");

	toolbar->AddControl(intensity_label);
	toolbar->AddControl(light_slider);
	toolbar->AddSeparator();
	toolbar->AddControl(ambient_label);
	toolbar->AddControl(ambient_slider);
	toolbar->AddSeparator();

	wxBitmap light_bitmap = IMAGE_MANAGER.GetBitmap(ICON_SUNNY, icon_size, wxColour(255, 235, 59));
	toolbar->AddTool(ID_LIGHT_TOGGLE, "Toggle Lighting", light_bitmap, "Toggle Lighting", wxITEM_CHECK);
	toolbar->ToggleTool(ID_LIGHT_TOGGLE, g_settings.getBoolean(Config::SHOW_LIGHTS));

	toolbar->Realize();

	light_slider->Bind(wxEVT_SLIDER, &LightToolBar::OnLightSlider, this);
	ambient_slider->Bind(wxEVT_SLIDER, &LightToolBar::OnAmbientLightSlider, this);
	toolbar->Bind(wxEVT_TOOL, &LightToolBar::OnToggleLight, this, ID_LIGHT_TOGGLE);
}

LightToolBar::~LightToolBar() {
	light_slider->Unbind(wxEVT_SLIDER, &LightToolBar::OnLightSlider, this);
	ambient_slider->Unbind(wxEVT_SLIDER, &LightToolBar::OnAmbientLightSlider, this);
	if (toolbar) {
		toolbar->Unbind(wxEVT_TOOL, &LightToolBar::OnToggleLight, this, ID_LIGHT_TOGGLE);
	}
}

void LightToolBar::OnLightSlider(wxCommandEvent& event) {
	g_gui.SetLightIntensity(event.GetInt() / 100.0f);
	g_gui.RefreshView();
}

void LightToolBar::OnAmbientLightSlider(wxCommandEvent& event) {
	g_gui.SetAmbientLightLevel(event.GetInt() / 100.0f);
	g_gui.RefreshView();
}

void LightToolBar::OnToggleLight(wxCommandEvent& event) {
	g_settings.setInteger(Config::SHOW_LIGHTS, event.IsChecked());
	g_gui.RefreshView();
}
