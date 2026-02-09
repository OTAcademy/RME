#ifndef RME_MAP_SESSION_H_
#define RME_MAP_SESSION_H_

#include "ui/gui_ids.h"

class Editor;

class BaseMap;

class MapSession {
public:
	MapSession(Editor* editor) :
		editor(editor),
		mode(SELECTION_MODE),
		secondary_map(nullptr) { }

	Editor* const editor;
	EditorMode mode;
	BaseMap* secondary_map;
};

#endif
