#ifndef RME_MAP_PROCESSOR_H
#define RME_MAP_PROCESSOR_H

class Editor;

class MapProcessor {
public:
	static void borderizeMap(Editor& editor, bool showdialog);
	static void randomizeMap(Editor& editor, bool showdialog);
	static void clearInvalidHouseTiles(Editor& editor, bool showdialog);
	static void clearModifiedTileState(Editor& editor, bool showdialog);
};

#endif
