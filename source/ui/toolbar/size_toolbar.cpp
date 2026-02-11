//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/size_toolbar.h"
#include "ui/gui.h"
#include "ui/gui_ids.h"
#include "editor/editor.h"
#include "util/image_manager.h"
#include <wx/artprov.h>

const wxString SizeToolBar::PANE_NAME = "sizes_toolbar";

SizeToolBar::SizeToolBar(wxWindow* parent) {
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));

	wxBitmap circular_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_4_SMALL, icon_size);
	wxBitmap rectangular_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_4_SMALL, icon_size);
	wxBitmap size1_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_1_SMALL, icon_size);
	wxBitmap size2_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_2_SMALL, icon_size);
	wxBitmap size3_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_3_SMALL, icon_size);
	wxBitmap size4_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_4_SMALL, icon_size);
	wxBitmap size5_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_5_SMALL, icon_size);
	wxBitmap size6_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_6_SMALL, icon_size);
	wxBitmap size7_bitmap = IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_7_SMALL, icon_size);

	toolbar = newd wxAuiToolBar(parent, TOOLBAR_SIZES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	toolbar->SetToolBitmapSize(icon_size);
	toolbar->AddTool(TOOLBAR_SIZES_RECTANGULAR, wxEmptyString, rectangular_bitmap, wxNullBitmap, wxITEM_CHECK, "Rectangular Brush", wxEmptyString, nullptr);
	toolbar->AddTool(TOOLBAR_SIZES_CIRCULAR, wxEmptyString, circular_bitmap, wxNullBitmap, wxITEM_CHECK, "Circular Brush", wxEmptyString, nullptr);
	toolbar->AddSeparator();
	toolbar->AddTool(TOOLBAR_SIZES_1, wxEmptyString, size1_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 1", wxEmptyString, nullptr);
	toolbar->AddTool(TOOLBAR_SIZES_2, wxEmptyString, size2_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 2", wxEmptyString, nullptr);
	toolbar->AddTool(TOOLBAR_SIZES_3, wxEmptyString, size3_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 3", wxEmptyString, nullptr);
	toolbar->AddTool(TOOLBAR_SIZES_4, wxEmptyString, size4_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 4", wxEmptyString, nullptr);
	toolbar->AddTool(TOOLBAR_SIZES_5, wxEmptyString, size5_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 5", wxEmptyString, nullptr);
	toolbar->AddTool(TOOLBAR_SIZES_6, wxEmptyString, size6_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 6", wxEmptyString, nullptr);
	toolbar->AddTool(TOOLBAR_SIZES_7, wxEmptyString, size7_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 7", wxEmptyString, nullptr);
	toolbar->Realize();
	toolbar->ToggleTool(TOOLBAR_SIZES_RECTANGULAR, true);
	toolbar->ToggleTool(TOOLBAR_SIZES_1, true);

	toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &SizeToolBar::OnToolbarClick, this);
}

SizeToolBar::~SizeToolBar() {
	toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &SizeToolBar::OnToolbarClick, this);
}

void SizeToolBar::Update() {
	Editor* editor = g_gui.GetCurrentEditor();
	bool has_map = editor != nullptr;

	toolbar->EnableTool(TOOLBAR_SIZES_CIRCULAR, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_RECTANGULAR, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_1, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_2, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_3, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_4, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_5, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_6, has_map);
	toolbar->EnableTool(TOOLBAR_SIZES_7, has_map);

	toolbar->Refresh();
}

void SizeToolBar::UpdateBrushSize(BrushShape shape, int size) {
	if (shape == BRUSHSHAPE_CIRCLE) {
		toolbar->ToggleTool(TOOLBAR_SIZES_CIRCULAR, true);
		toolbar->ToggleTool(TOOLBAR_SIZES_RECTANGULAR, false);

		wxSize icon_size = wxSize(16, 16);
		toolbar->SetToolBitmap(TOOLBAR_SIZES_1, IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_1_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_2, IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_2_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_3, IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_3_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_4, IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_4_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_5, IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_5_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_6, IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_6_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_7, IMAGE_MANAGER.GetBitmap(IMAGE_CIRCULAR_7_SMALL, icon_size));
	} else {
		toolbar->ToggleTool(TOOLBAR_SIZES_CIRCULAR, false);
		toolbar->ToggleTool(TOOLBAR_SIZES_RECTANGULAR, true);

		wxSize icon_size = wxSize(16, 16);
		toolbar->SetToolBitmap(TOOLBAR_SIZES_1, IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_1_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_2, IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_2_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_3, IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_3_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_4, IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_4_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_5, IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_5_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_6, IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_6_SMALL, icon_size));
		toolbar->SetToolBitmap(TOOLBAR_SIZES_7, IMAGE_MANAGER.GetBitmap(IMAGE_RECTANGULAR_7_SMALL, icon_size));
	}

	toolbar->ToggleTool(TOOLBAR_SIZES_1, size == 0);
	toolbar->ToggleTool(TOOLBAR_SIZES_2, size == 1);
	toolbar->ToggleTool(TOOLBAR_SIZES_3, size == 2);
	toolbar->ToggleTool(TOOLBAR_SIZES_4, size == 4);
	toolbar->ToggleTool(TOOLBAR_SIZES_5, size == 6);
	toolbar->ToggleTool(TOOLBAR_SIZES_6, size == 8);
	toolbar->ToggleTool(TOOLBAR_SIZES_7, size == 11);

	g_gui.GetAuiManager()->Update();
}

void SizeToolBar::OnToolbarClick(wxCommandEvent& event) {
	if (!g_gui.IsEditorOpen()) {
		return;
	}

	switch (event.GetId()) {
		case TOOLBAR_SIZES_CIRCULAR:
			g_gui.SetBrushShape(BRUSHSHAPE_CIRCLE);
			break;
		case TOOLBAR_SIZES_RECTANGULAR:
			g_gui.SetBrushShape(BRUSHSHAPE_SQUARE);
			break;
		case TOOLBAR_SIZES_1:
			g_gui.SetBrushSize(0);
			break;
		case TOOLBAR_SIZES_2:
			g_gui.SetBrushSize(1);
			break;
		case TOOLBAR_SIZES_3:
			g_gui.SetBrushSize(2);
			break;
		case TOOLBAR_SIZES_4:
			g_gui.SetBrushSize(4);
			break;
		case TOOLBAR_SIZES_5:
			g_gui.SetBrushSize(6);
			break;
		case TOOLBAR_SIZES_6:
			g_gui.SetBrushSize(8);
			break;
		case TOOLBAR_SIZES_7:
			g_gui.SetBrushSize(11);
			break;
		default:
			break;
	}
}
