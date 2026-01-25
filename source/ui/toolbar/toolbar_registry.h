//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_TOOLBAR_TOOLBAR_REGISTRY_H_
#define RME_UI_TOOLBAR_TOOLBAR_REGISTRY_H_

#include <wx/wx.h>
#include <wx/aui/aui.h>
#include <memory>

#include "ui/gui_ids.h"
#include "ui/toolbar/standard_toolbar.h"
#include "ui/toolbar/brush_toolbar.h"
#include "ui/toolbar/position_toolbar.h"
#include "ui/toolbar/size_toolbar.h"
#include "ui/toolbar/light_toolbar.h"

// Responsibility: Holds pointers to all toolbar components and manages their lifecycle.
class ToolbarRegistry {
public:
	ToolbarRegistry();
	~ToolbarRegistry();

	void SetStandardToolbar(std::unique_ptr<StandardToolBar> tb);
	void SetBrushToolbar(std::unique_ptr<BrushToolBar> tb);
	void SetPositionToolbar(std::unique_ptr<PositionToolBar> tb);
	void SetSizeToolbar(std::unique_ptr<SizeToolBar> tb);
	void SetLightToolbar(std::unique_ptr<LightToolBar> tb);

	StandardToolBar* GetStandardToolbar() const {
		return standard_toolbar.get();
	}
	BrushToolBar* GetBrushToolbar() const {
		return brush_toolbar.get();
	}
	PositionToolBar* GetPositionToolbar() const {
		return position_toolbar.get();
	}
	SizeToolBar* GetSizeToolbar() const {
		return size_toolbar.get();
	}
	LightToolBar* GetLightToolbar() const {
		return light_toolbar.get();
	}

	wxAuiPaneInfo& GetPane(ToolBarID id, wxAuiManager* manager);

	void UpdateAll();

private:
	// Owned pointers
	std::unique_ptr<StandardToolBar> standard_toolbar;
	std::unique_ptr<BrushToolBar> brush_toolbar;
	std::unique_ptr<PositionToolBar> position_toolbar;
	std::unique_ptr<SizeToolBar> size_toolbar;
	std::unique_ptr<LightToolBar> light_toolbar;
};

#endif // RME_UI_TOOLBAR_TOOLBAR_REGISTRY_H_
