//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_EDITOR_MANAGER_H_
#define RME_EDITOR_MANAGER_H_

#include "app/main.h"

class Map;
class Editor;
class EditorTab;
class MapTab;
class LiveSocket;

class EditorManager {
public:
	EditorManager();
	~EditorManager();

	EditorTab* GetCurrentTab();
	EditorTab* GetTab(int idx);
	int GetTabCount() const;
	bool IsAnyEditorOpen() const;
	bool IsEditorOpen() const;
	void CloseCurrentEditor();
	Editor* GetCurrentEditor();
	MapTab* GetCurrentMapTab() const;
	void CycleTab(bool forward = true);
	bool CloseLiveEditors(LiveSocket* sock);
	bool CloseAllEditors();
	void NewMapView();

	// Map lifecycle
	Map& GetCurrentMap();
	int GetOpenMapCount();
	bool ShouldSave();
	void SaveCurrentMap(FileName filename, bool showdialog);
	bool NewMap();
	void OpenMap();
	void SaveMap();
	void SaveMapAs();
	bool LoadMap(const FileName& fileName);

	// Edit operations
	void DoCut();
	void DoCopy();
	void DoPaste();
	void PreparePaste();
	void StartPasting();
	void EndPasting();

	bool CanUndo();
	bool CanRedo();
	bool DoUndo();
	bool DoRedo();
};

extern EditorManager g_editors;

#endif
