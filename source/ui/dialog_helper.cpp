//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include <wx/wx.h>

#include "ui/dialog_helper.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "map/tile.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "ui/properties/creature_properties_window.h"
#include "ui/properties/spawn_properties_window.h"
#include "ui/properties/container_properties_window.h"
#include "ui/properties/podium_properties_window.h"
#include "ui/properties/writable_properties_window.h"
#include "ui/properties/splash_properties_window.h"
#include "ui/properties/depot_properties_window.h"
#include "ui/properties/old_properties_window.h"
#include "ui/properties/properties_window.h"

void DialogHelper::OpenProperties(Editor& editor, Tile* tile) {
	if (!tile) {
		return;
	}

	std::unique_ptr<Tile> new_tile = tile->deepCopy(editor.map);
	wxDialog* w = nullptr;

	if (new_tile->spawn && g_settings.getInteger(Config::SHOW_SPAWNS)) {
		w = newd SpawnPropertiesWindow(g_gui.root, &editor.map, new_tile.get(), new_tile->spawn.get());
	} else if (new_tile->creature && g_settings.getInteger(Config::SHOW_CREATURES)) {
		w = newd CreaturePropertiesWindow(g_gui.root, &editor.map, new_tile.get(), new_tile->creature.get());
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
			if (dynamic_cast<Container*>(item)) {
				w = newd ContainerPropertiesWindow(g_gui.root, &editor.map, new_tile.get(), item);
			} else if (dynamic_cast<Podium*>(item)) {
				w = newd PodiumPropertiesWindow(g_gui.root, &editor.map, new_tile.get(), item);
			} else if (editor.map.getVersion().otbm < MAP_OTBM_4) {
				if (item->canHoldText() || item->canHoldDescription()) {
					w = newd WritablePropertiesWindow(g_gui.root, &editor.map, new_tile.get(), item);
				} else if (item->isSplash() || item->isFluidContainer()) {
					w = newd SplashPropertiesWindow(g_gui.root, &editor.map, new_tile.get(), item);
				} else if (dynamic_cast<Depot*>(item)) {
					w = newd DepotPropertiesWindow(g_gui.root, &editor.map, new_tile.get(), item);
				} else {
					w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile.get(), item);
				}
			} else {
				w = newd PropertiesWindow(g_gui.root, &editor.map, new_tile.get(), item);
			}
		}
	}

	if (w) {
		int ret = w->ShowModal();
		if (ret != 0) {
			std::unique_ptr<Action> action = editor.actionQueue->createAction(ACTION_CHANGE_PROPERTIES);
			action->addChange(std::make_unique<Change>(new_tile.release()));
			editor.addAction(std::move(action));
		}
		w->Destroy();
	}
}
