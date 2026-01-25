//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_ObjectPropertiesWindowBase_H
#define RME_ObjectPropertiesWindowBase_H

#include "app/main.h"

class Map;
class Tile;
class Item;
class Creature;
class Spawn;

/**
 * Base for the item properties dialogs
 * There are two versions, one for otbmv4 maps and one for the old maps.
 * They are declared in old_properties_window / properties_window
 */
class ObjectPropertiesWindowBase : public wxDialog {
public:
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title,
		const Map* map, const Tile* tile, Item* item,
		wxPoint position = wxDefaultPosition
	);
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title,
		const Map* map, const Tile* tile, Spawn* spawn,
		wxPoint position = wxDefaultPosition
	);
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title,
		const Map* map, const Tile* tile, Creature* creature,
		wxPoint position = wxDefaultPosition
	);
	ObjectPropertiesWindowBase(
		wxWindow* parent, wxString title, wxPoint position = wxDefaultPosition
	);

	Item* getItemBeingEdited();
	Creature* getCreatureBeingEdited();
	Spawn* getSpawnBeingEdited();

protected:
	const Map* edit_map;
	const Tile* edit_tile;
	Item* edit_item;
	Creature* edit_creature;
	Spawn* edit_spawn;
};

#endif
