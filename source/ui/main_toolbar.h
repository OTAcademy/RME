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

#ifndef RME_MAINTOOLBAR_H_
#define RME_MAINTOOLBAR_H_

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <wx/aui/auibar.h>

#include "ui/gui_ids.h"
#include "brushes/brush_enums.h"
#include "ui/numbertextctrl.h"

#include "ui/toolbar/brush_toolbar.h"
#include "ui/toolbar/position_toolbar.h"
#include "ui/toolbar/size_toolbar.h"
#include "ui/toolbar/standard_toolbar.h"
#include "ui/toolbar/light_toolbar.h"
#include "ui/toolbar/toolbar_registry.h"
#include <memory>

class MainToolBar : public wxEvtHandler {
public:
	MainToolBar(wxWindow* parent, wxAuiManager* manager);
	~MainToolBar();

	wxAuiPaneInfo& GetPane(ToolBarID id);
	void UpdateButtons();
	void UpdateBrushButtons();
	void UpdateBrushSize(BrushShape shape, int size);
	void Show(ToolBarID id, bool show);
	void HideAll(bool update = true);
	void LoadPerspective();
	void SavePerspective();

	void OnStandardButtonClick(wxCommandEvent& event);
	void OnLightSlider(wxCommandEvent& event);
	void OnAmbientLightSlider(wxCommandEvent& event);

private:
	static const wxString STANDARD_BAR_NAME;
	static const wxString LIGHT_BAR_NAME;

	StandardToolBar* GetStandardToolbar() {
		return registry->GetStandardToolbar();
	}
	BrushToolBar* GetBrushToolbar() {
		return registry->GetBrushToolbar();
	}
	PositionToolBar* GetPositionToolbar() {
		return registry->GetPositionToolbar();
	}
	SizeToolBar* GetSizeToolbar() {
		return registry->GetSizeToolbar();
	}
	LightToolBar* GetLightToolbar() {
		return registry->GetLightToolbar();
	}

private:
	std::unique_ptr<ToolbarRegistry> registry;
};

#endif // RME_MAINTOOLBAR_H_
