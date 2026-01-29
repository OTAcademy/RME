//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/managers/layout_manager.h"
#include "app/managers/version_manager.h"
#include "ui/gui.h"
#include "app/application.h"
#include "palette/palette_window.h"
#include "palette/house/house_palette.h"
#include "palette/managers/palette_manager.h"

#include "rendering/ui/minimap_window.h"
#include "ui/main_menubar.h"
#include "ui/main_toolbar.h"
#include <wx/display.h>
#include "ui/managers/minimap_manager.h"
#include "ingame_preview/ingame_preview_manager.h"
#include "ingame_preview/ingame_preview_window.h"
#include "ui/managers/status_manager.h"
#include "ui/tool_options_window.h"

LayoutManager g_layout;

LayoutManager::LayoutManager() {
}

LayoutManager::~LayoutManager() {
}

void LayoutManager::LoadPerspective() {
	if (!g_version.IsVersionLoaded()) {
		if (g_settings.getInteger(Config::WINDOW_MAXIMIZED)) {
			g_gui.root->Maximize();
		} else {
			g_gui.root->SetSize(wxSize(
				g_settings.getInteger(Config::WINDOW_WIDTH),
				g_settings.getInteger(Config::WINDOW_HEIGHT)
			));
		}
	} else {
		std::string tmp;
		std::string layout = g_settings.getString(Config::PALETTE_LAYOUT);

		std::vector<std::string> palette_list;
		for (char c : layout) {
			if (c == '|') {
				palette_list.push_back(tmp);
				tmp.clear();
			} else {
				tmp.push_back(c);
			}
		}

		if (!tmp.empty()) {
			palette_list.push_back(tmp);
		}

		for (const std::string& name : palette_list) {
			PaletteWindow* palette = g_gui.CreatePalette();

			wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(palette);
			g_gui.aui_manager->LoadPaneInfo(wxstr(name), info);

			if (info.IsFloatable()) {
				bool offscreen = true;
				for (uint32_t index = 0; index < wxDisplay::GetCount(); ++index) {
					wxDisplay display(index);
					wxRect rect = display.GetClientArea();
					if (rect.Contains(info.floating_pos)) {
						offscreen = false;
						break;
					}
				}

				if (offscreen) {
					info.Dock();
				}
			}
		}

		if (g_settings.getInteger(Config::MINIMAP_VISIBLE)) {
			if (!g_minimap.GetWindow()) {
				wxAuiPaneInfo info;

				const wxString& data = wxstr(g_settings.getString(Config::MINIMAP_LAYOUT));
				g_gui.aui_manager->LoadPaneInfo(data, info);

				g_minimap.SetWindow(newd MinimapWindow(g_gui.root));
				g_gui.aui_manager->AddPane(g_minimap.GetWindow(), info);
			} else {
				wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(g_minimap.GetWindow());

				const wxString& data = wxstr(g_settings.getString(Config::MINIMAP_LAYOUT));
				g_gui.aui_manager->LoadPaneInfo(data, info);
			}

			wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(g_minimap.GetWindow());
			if (info.IsFloatable()) {
				bool offscreen = true;
				for (uint32_t index = 0; index < wxDisplay::GetCount(); ++index) {
					wxDisplay display(index);
					wxRect rect = display.GetClientArea();
					if (rect.Contains(info.floating_pos)) {
						offscreen = false;
						break;
					}
				}

				if (offscreen) {
					info.Dock();
				}
			}
		}

		g_gui.aui_manager->Update();
		g_status.UpdateTitle();
		g_gui.root->UpdateMenubar();
	}

	// Initialize ToolOptionsWindow
	if (!g_gui.tool_options) {
		g_gui.tool_options = newd ToolOptionsWindow(g_gui.root);

		wxAuiPaneInfo info;
		wxString layout = wxstr(g_settings.getString(Config::TOOL_OPTIONS_LAYOUT));
		if (!layout.empty()) {
			g_gui.aui_manager->LoadPaneInfo(layout, info);
		} else {
			info.Name("ToolOptions").Caption("Tool Options").Right().Layer(0).Position(0).CloseButton(true).MaximizeButton(true).BestSize(230, 300);
		}
		if (info.name.empty()) {
			info.Name("ToolOptions");
		}

		g_gui.aui_manager->AddPane(g_gui.tool_options, info);
	} else {
		wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(g_gui.tool_options);
		wxString layout = wxstr(g_settings.getString(Config::TOOL_OPTIONS_LAYOUT));
		if (!layout.empty()) {
			g_gui.aui_manager->LoadPaneInfo(layout, info);
		}
	}

	if (g_settings.getInteger(Config::INGAME_PREVIEW_VISIBLE)) {
		g_preview.Create();
		if (g_preview.GetWindow()) {
			wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(g_preview.GetWindow());
			wxString layout = wxstr(g_settings.getString(Config::INGAME_PREVIEW_LAYOUT));
			if (!layout.empty()) {
				g_gui.aui_manager->LoadPaneInfo(layout, info);
			}
		}
	}

	if (!g_gui.house_palette) {
		g_gui.house_palette = newd HousePalette(g_gui.root);
		wxAuiPaneInfo info;
		wxString layout = wxstr(g_settings.getString(Config::HOUSE_PALETTE_LAYOUT));
		if (!layout.empty()) {
			g_gui.aui_manager->LoadPaneInfo(layout, info);
		} else {
			info.Name("HousePalette").Caption("Houses").DefaultPane().Right().Layer(0).Position(1).CloseButton(true).MaximizeButton(true).BestSize(230, 400);
		}
		if (info.name.empty()) {
			info.Name("HousePalette");
		}
		g_gui.aui_manager->AddPane(g_gui.house_palette, info);
	} else {
		wxAuiPaneInfo& info = g_gui.aui_manager->GetPane(g_gui.house_palette);
		wxString layout = wxstr(g_settings.getString(Config::HOUSE_PALETTE_LAYOUT));
		if (!layout.empty()) {
			g_gui.aui_manager->LoadPaneInfo(layout, info);
		}
	}

	g_gui.aui_manager->Update();
	g_gui.root->GetAuiToolBar()->LoadPerspective();
}

void LayoutManager::SavePerspective() {
	g_settings.setInteger(Config::WINDOW_MAXIMIZED, g_gui.root->IsMaximized());
	g_settings.setInteger(Config::WINDOW_WIDTH, g_gui.root->GetSize().GetWidth());
	g_settings.setInteger(Config::WINDOW_HEIGHT, g_gui.root->GetSize().GetHeight());

	g_settings.setInteger(Config::MINIMAP_VISIBLE, g_minimap.GetWindow() ? 1 : 0);

	wxString pinfo;
	for (auto& palette : g_palettes.palettes) {
		if (g_gui.aui_manager->GetPane(palette).IsShown()) {
			pinfo << g_gui.aui_manager->SavePaneInfo(g_gui.aui_manager->GetPane(palette)) << "|";
		}
	}
	g_settings.setString(Config::PALETTE_LAYOUT, nstr(pinfo));

	if (g_minimap.GetWindow()) {
		wxString s = g_gui.aui_manager->SavePaneInfo(g_gui.aui_manager->GetPane(g_minimap.GetWindow()));
		g_settings.setString(Config::MINIMAP_LAYOUT, nstr(s));
	}

	if (g_gui.tool_options) {
		wxString s = g_gui.aui_manager->SavePaneInfo(g_gui.aui_manager->GetPane(g_gui.tool_options));
		g_settings.setString(Config::TOOL_OPTIONS_LAYOUT, nstr(s));
	}

	g_settings.setInteger(Config::INGAME_PREVIEW_VISIBLE, g_preview.IsVisible() ? 1 : 0);
	if (g_preview.GetWindow()) {
		wxString s = g_gui.aui_manager->SavePaneInfo(g_gui.aui_manager->GetPane(g_preview.GetWindow()));
		g_settings.setString(Config::INGAME_PREVIEW_LAYOUT, nstr(s));
	}

	if (g_gui.house_palette) {
		wxString s = g_gui.aui_manager->SavePaneInfo(g_gui.aui_manager->GetPane(g_gui.house_palette));
		g_settings.setString(Config::HOUSE_PALETTE_LAYOUT, nstr(s));
	}

	g_gui.root->GetAuiToolBar()->SavePerspective();
}
