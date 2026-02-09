#ifndef RME_GUI_H_
#define RME_GUI_H_

#include "rendering/core/graphics.h"
#include "map/position.h"

#include "editor/copybuffer.h"
#include "ui/dcbutton.h"
#include "brushes/brush_enums.h"
#include "ui/gui_ids.h"
#include "editor/editor_tabs.h"
#include "ui/map_tab.h"
#include "palette/palette_window.h"
#include "app/client_version.h"
#include "app/managers/version_manager.h"
#include "ui/managers/loading_manager.h"
#include "ui/managers/layout_manager.h"
#include "ui/managers/minimap_manager.h"
#include "brushes/managers/doodad_preview_manager.h"
#include "brushes/managers/autoborder_preview_manager.h"
#include "ui/managers/status_manager.h"
#include "ui/managers/search_manager.h"
#include "ui/managers/welcome_manager.h"
#include "ui/managers/gl_context_manager.h"

class BaseMap;
class Map;

class Editor;
class Brush;
class HouseBrush;
class HouseExitBrush;
class WaypointBrush;
class OptionalBorderBrush;
class EraserBrush;
class SpawnBrush;
class DoorBrush;
class FlagBrush;

class MainFrame;
class WelcomeDialog;
class MapWindow;
class MapCanvas;
class LiveClient;
class SearchResultWindow;
class MinimapWindow;
class PaletteWindow;
class OldPropertiesWindow;
class TilesetWindow;
class EditTownsDialog;
class ItemButton;
class HousePalette;

class LiveSocket;

class SidebarWindow;
class ToolOptionsWindow;

wxDECLARE_EVENT(EVT_UPDATE_MENUS, wxCommandEvent);

#define EVT_ON_UPDATE_MENUS(id, fn)                                                             \
	DECLARE_EVENT_TABLE_ENTRY(                                                                  \
		EVT_UPDATE_MENUS, id, wxID_ANY,                                                         \
		(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxCommandEventFunction, &fn), \
		(wxObject*)nullptr                                                                      \
	),

#include <mutex>
#include "brushes/managers/brush_manager.h"
#include "palette/managers/palette_manager.h"
#include "editor/managers/editor_manager.h"
#include "editor/hotkey_manager.h"

class GUI {
public: // dtor and ctor
	GUI();
	~GUI();

private:
	GUI(const GUI& g_gui); // Don't copy me
	GUI& operator=(const GUI& g_gui); // Don't assign me
	bool operator==(const GUI& g_gui); // Don't compare me

public:
	/**
	 * Saves the perspective to the configuration file
	 * This is the position of all windows etc. in the editor
	 */
	void SavePerspective() {
		g_layout.SavePerspective();
	}

	/**
	 * Loads the stored perspective from the configuration file
	 */
	void LoadPerspective() {
		g_layout.LoadPerspective();
	}

	void CreateLoadBar(wxString message, bool canCancel = false) {
		g_loading.CreateLoadBar(message, canCancel);
	}
	void SetLoadScale(int32_t from, int32_t to) {
		g_loading.SetLoadScale(from, to);
	}
	bool SetLoadDone(int32_t done, const wxString& newMessage = "") {
		return g_loading.SetLoadDone(done, newMessage);
	}

	void ShowWelcomeDialog(const wxBitmap& icon) {
		g_welcome.ShowWelcomeDialog(icon);
	}
	void FinishWelcomeDialog() {
		g_welcome.FinishWelcomeDialog();
	}
	bool IsWelcomeDialogShown() {
		return g_welcome.IsWelcomeDialogShown();
	}

	/**
	 * Destroys (hides) the current loading bar.
	 */

	void DestroyLoadBar() {
		g_loading.DestroyLoadBar();
	}

	void UpdateMenubar();

	bool IsRenderingEnabled() const {
		return disabled_counter == 0;
	}

	// Hotkey methods removed (moved to HotkeyManager)

	// This sends the event to the main window (redirecting from other controls)
	void AddPendingCanvasEvent(wxEvent& event);

protected:
	void DisableRendering() {
		++disabled_counter;
	}
	void EnableRendering() {
		--disabled_counter;
	}

public:
	void SetTitle(wxString newtitle) {
		g_status.SetTitle(newtitle);
	}
	void UpdateTitle() {
		g_status.UpdateTitle();
	}
	void UpdateMenus();
	void ShowToolbar(ToolBarID id, bool show);
	void SetStatusText(wxString text) {
		g_status.SetStatusText(text);
	}

	// Get the current GL context
	// Param is required if the context is to be created.
	wxGLContext* GetGLContext(wxGLCanvas* win) {
		return g_gl_context.GetGLContext(win);
	}

	// Search Results
	SearchResultWindow* ShowSearchWindow() {
		return g_search.ShowSearchWindow();
	}
	void HideSearchWindow() {
		g_search.HideSearchWindow();
	}

	// Minimap
	void CreateMinimap() {
		g_minimap.Create();
	}
	void HideMinimap() {
		g_minimap.Hide();
	}
	void DestroyMinimap() {
		g_minimap.Destroy();
	}
	void UpdateMinimap(bool immediate = false) {
		g_minimap.Update(immediate);
	}
	bool IsMinimapVisible() const {
		return g_minimap.IsVisible();
	}

	int GetCurrentFloor();
	void ChangeFloor(int newfloor);

	double GetCurrentZoom();
	void SetCurrentZoom(double zoom);

	void SwitchMode();
	void SetSelectionMode();
	void SetDrawingMode();
	bool IsSelectionMode() const;
	bool IsDrawingMode() const;

	// Brushes
	void FillDoodadPreviewBuffer();
	void UpdateAutoborderPreview(Position pos);
	// Selects the currently seleceted brush in the active palette
	void SelectBrush();
	// Updates the palette AND selects the brush, second parameter is first palette to look in
	// Returns true if the brush was found and selected
	bool SelectBrush(const Brush* brush, PaletteType pt = TILESET_UNKNOWN);
	// Selects the brush selected before the current brush
	void SelectPreviousBrush();
	// Only selects the brush, doesn't update the palette
	void SelectBrushInternal(Brush* brush);
	// Get different brush parameters
	Brush* GetCurrentBrush() const;
	BrushShape GetBrushShape() const;
	int GetBrushSize() const;
	int GetBrushVariation() const;
	int GetSpawnTime() const;

	// Additional brush parameters
	void SetSpawnTime(int time);

	void SetLightIntensity(float v);
	float GetLightIntensity() const;

	void SetAmbientLightLevel(float v);
	float GetAmbientLightLevel() const;
	void SetBrushSize(int nz);
	void SetBrushSizeInternal(int nz);
	void SetBrushShape(BrushShape bs);
	void SetBrushVariation(int nz);
	void SetBrushThickness(int low, int ceil);
	void SetBrushThickness(bool on, int low = -1, int ceil = -1);
	// Helper functions for size
	void DecreaseBrushSize(bool wrap = false);
	void IncreaseBrushSize(bool wrap = false);
	// Door brush options
	void SetDoorLocked(bool on);
	bool HasDoorLocked();

	// Centers current view on position
	void SetScreenCenterPosition(Position pos);
	// Refresh the view canvas
	void RefreshView();
	// Fit all/specified current map view to map dimensions
	void FitViewToMap();
	void FitViewToMap(MapTab* mt);

	void DoCut();
	void DoCopy();
	void DoPaste();
	void PreparePaste();
	void StartPasting();
	void EndPasting();
	bool IsPasting() const {
		return pasting;
	}

	bool CanUndo();
	bool CanRedo();
	bool DoUndo();
	bool DoRedo();

	// Editor interface
	wxAuiManager* GetAuiManager() const {
		return aui_manager.get();
	}
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
	void AddPendingLiveClient(std::unique_ptr<LiveClient> client);
	std::unique_ptr<LiveClient> PopPendingLiveClient(LiveClient* ptr);

	// Map
	Map& GetCurrentMap();
	int GetOpenMapCount();
	bool ShouldSave();
	void SaveCurrentMap(FileName filename, bool showdialog);
	void SaveCurrentMap(bool showdialog = true);
	bool NewMap();
	void OpenMap();
	void SaveMap();
	void SaveMapAs();
	bool LoadMap(const FileName& fileName);

protected:
	ClientVersion* getLoadedVersion() const {
		return g_version.getLoadedVersion();
	}

	//=========================================================================
	// Palette Interface
	//=========================================================================
public:
	// Spawn a newd palette
	PaletteWindow* NewPalette();
	// Bring this palette to the front (as the 'active' palette)
	void ActivatePalette(PaletteWindow* p);
	// Rebuild forces palette to reload the entire contents
	void RebuildPalettes();
	// Refresh only updates the content (such as house/waypoint list)
	void RefreshPalettes(Map* m = nullptr, bool usedfault = true);
	// Won't refresh the palette in the parameter
	void RefreshOtherPalettes(PaletteWindow* p);
	// If no palette is shown, this displays the primary palette
	// else does nothing.
	void ShowPalette();
	// Select a particular page on the primary palette
	void SelectPalettePage(PaletteType pt);

	// Returns primary palette
	PaletteWindow* GetPalette();
	// Returns list of all palette, first in the list is primary
	const std::list<PaletteWindow*>& GetPalettes();

	void DestroyPalettes();
	PaletteWindow* CreatePalette();

protected:
	//=========================================================================
	// Public members
	//=========================================================================
public:
	std::unique_ptr<wxAuiManager> aui_manager;
	std::unique_ptr<MapTabbook> tabbook;
	MainFrame* root; // The main frame
	CopyBuffer copybuffer;

	GraphicManager gfx;

	HousePalette* house_palette;

	ToolOptionsWindow* tool_options;

	bool pasting;

	Hotkey hotkeys[10];
	bool hotkeys_enabled;

protected:
	wxWindowDisabler* winDisabler;

	int disabled_counter;

	std::mutex pending_live_clients_mutex;
	std::vector<std::unique_ptr<LiveClient>> pending_live_clients;

	friend class RenderingLock;
	friend class MapTab;
};

extern GUI g_gui;

class RenderingLock {
	bool acquired;

public:
	RenderingLock() :
		acquired(true) {
		g_gui.DisableRendering();
	}
	~RenderingLock() {
		release();
	}
	void release() {
		g_gui.EnableRendering();
		acquired = false;
	}
};

/**
 * Will push a loading bar when it is constructed
 * which will the be popped when it destructs.
 * Look in the GUI class for documentation of what the methods mean.
 */
class ScopedLoadingBar {
public:
	ScopedLoadingBar(wxString message, bool canCancel = false) {
		g_loading.CreateLoadBar(message, canCancel);
	}
	~ScopedLoadingBar() {
		g_loading.DestroyLoadBar();
	}

	void SetLoadDone(int32_t done, const wxString& newmessage = wxEmptyString) {
		g_loading.SetLoadDone(done, newmessage);
	}

	void SetLoadScale(int32_t from, int32_t to) {
		g_loading.SetLoadScale(from, to);
	}
};

#define UnnamedRenderingLock() RenderingLock __unnamed_rendering_lock_##__LINE__

void SetWindowToolTip(wxWindow* a, const wxString& tip);
void SetWindowToolTip(wxWindow* a, wxWindow* b, const wxString& tip);

#endif
