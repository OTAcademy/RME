//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include <wx/wx.h>

#include "dialog_helper.h"
#include "editor.h"
#include "tile.h"
#include "gui.h"
#include "settings.h"
#include "old_properties_window.h"
#include "properties_window.h"

void DialogHelper::OpenProperties(Editor& editor, Tile* tile) {
	if (!tile) {
		return;
	}

	Tile* new_tile = tile->deepCopy(editor.map);
	wxDialog* w = nullptr;

	if (new_tile->spawn && g_settings.getInteger(Config::SHOW_SPAWNS)) {
		w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->spawn);
	} else if (new_tile->creature && g_settings.getInteger(Config::SHOW_CREATURES)) {
		w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->creature);
	} else {
		ItemVector selected_items = new_tile->getSelectedItems();
		Item* item = nullptr;

		for (auto* it : selected_items) {
			if (it->isSelected()) {
				item = it;
				break;
			}
		}

		if (!item) {
			item = new_tile->getTopItem();
		}

		if (item) {
			if (editor.map.getVersion().otbm >= MAP_OTBM_4) {
				w = newd PropertiesWindow(g_gui.root, &editor.map, new_tile, item);
			} else {
				w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, item);
			}
		}
	}

	if (w) {
		int ret = w->ShowModal();
		if (ret != 0) {
			Action* action = editor.actionQueue->createAction(ACTION_CHANGE_PROPERTIES);
			action->addChange(newd Change(new_tile));
			editor.addAction(action);
		} else {
			delete new_tile;
		}
		w->Destroy();
	} else {
		delete new_tile;
	}
}
