//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "ui/toolbar/toolbar_persistence.h"
#include "app/settings.h"
#include "ui/gui_ids.h"
#include "ui/toolbar/brush_toolbar.h"
#include "ui/toolbar/position_toolbar.h"
#include "ui/toolbar/size_toolbar.h"
#include "ui/toolbar/standard_toolbar.h"

void ToolbarPersistence::LoadPerspective(wxAuiManager* manager, MainToolBar* mainToolbar) {
	if (!manager || !mainToolbar) {
		return;
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD)) {
		std::string info = g_settings.getString(Config::TOOLBAR_STANDARD_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), mainToolbar->GetPane(TOOLBAR_STANDARD));
		}
		mainToolbar->GetPane(TOOLBAR_STANDARD).Show();
	} else {
		mainToolbar->GetPane(TOOLBAR_STANDARD).Hide();
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		std::string info = g_settings.getString(Config::TOOLBAR_BRUSHES_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), mainToolbar->GetPane(TOOLBAR_BRUSHES));
		}
		mainToolbar->GetPane(TOOLBAR_BRUSHES).Show();
	} else {
		mainToolbar->GetPane(TOOLBAR_BRUSHES).Hide();
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION)) {
		std::string info = g_settings.getString(Config::TOOLBAR_POSITION_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), mainToolbar->GetPane(TOOLBAR_POSITION));
		}
		mainToolbar->GetPane(TOOLBAR_POSITION).Show();
	} else {
		mainToolbar->GetPane(TOOLBAR_POSITION).Hide();
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES)) {
		std::string info = g_settings.getString(Config::TOOLBAR_SIZES_LAYOUT);
		if (!info.empty()) {
			manager->LoadPaneInfo(wxString(info), mainToolbar->GetPane(TOOLBAR_SIZES));
		}
		mainToolbar->GetPane(TOOLBAR_SIZES).Show();
	} else {
		mainToolbar->GetPane(TOOLBAR_SIZES).Hide();
	}

	manager->Update();
}

void ToolbarPersistence::SavePerspective(wxAuiManager* manager, MainToolBar* mainToolbar) {
	if (!manager || !mainToolbar) {
		return;
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD)) {
		wxString info = manager->SavePaneInfo(mainToolbar->GetPane(TOOLBAR_STANDARD));
		g_settings.setString(Config::TOOLBAR_STANDARD_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		wxString info = manager->SavePaneInfo(mainToolbar->GetPane(TOOLBAR_BRUSHES));
		g_settings.setString(Config::TOOLBAR_BRUSHES_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION)) {
		wxString info = manager->SavePaneInfo(mainToolbar->GetPane(TOOLBAR_POSITION));
		g_settings.setString(Config::TOOLBAR_POSITION_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES)) {
		wxString info = manager->SavePaneInfo(mainToolbar->GetPane(TOOLBAR_SIZES));
		g_settings.setString(Config::TOOLBAR_SIZES_LAYOUT, info.ToStdString());
	}
}
