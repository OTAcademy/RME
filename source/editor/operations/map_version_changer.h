#ifndef RME_EDITOR_OPERATIONS_MAP_VERSION_CHANGER_H
#define RME_EDITOR_OPERATIONS_MAP_VERSION_CHANGER_H

#include "map/map.h"
#include <wx/window.h>

class Editor;

class MapVersionChanger {
public:
	// Returns true if conversion was successful or unnecessary
	static bool changeMapVersion(wxWindow* parent, Editor& editor, MapVersion new_ver);
};

#endif
