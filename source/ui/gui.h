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

class SearchResultWindow;
class MinimapWindow;
class PaletteWindow;
class OldPropertiesWindow;
class TilesetWindow;
class EditTownsDialog;
class ItemButton;

class LiveSocket;

extern const wxEventType EVT_UPDATE_MENUS;

#define EVT_ON_UPDATE_MENUS(id, fn)                                                             \
	DECLARE_EVENT_TABLE_ENTRY(                                                                  \
		EVT_UPDATE_MENUS, id, wxID_ANY,                                                         \
		(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(wxCommandEventFunction, &fn), \
		(wxObject*)nullptr                                                                      \
	),

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

	void ShowWelcomeDialog(const wxBitmap& icon);

	void FinishWelcomeDialog();
	bool IsWelcomeDialogShown();

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

	void OnWelcomeDialogClosed(wxCloseEvent& event);
	void OnWelcomeDialogAction(wxCommandEvent& event);

protected:
	void DisableRendering() {
		++disabled_counter;
	}
	void EnableRendering() {
		--disabled_counter;
	}

public:
	void SetTitle(wxString newtitle);
	void UpdateTitle();
	void UpdateMenus();
	void ShowToolbar(ToolBarID id, bool show);
	void SetStatusText(wxString text);

	// Get the current GL context
	// Param is required if the context is to be created.
	wxGLContext* GetGLContext(wxGLCanvas* win);

	// Search Results
	SearchResultWindow* ShowSearchWindow();
	void HideSearchWindow();

	// Minimap
	void CreateMinimap();
	void HideMinimap();
	void DestroyMinimap();
	void UpdateMinimap(bool immediate = false);
	bool IsMinimapVisible() const;

	int GetCurrentFloor();
	void ChangeFloor(int newfloor);

	double GetCurrentZoom();
	void SetCurrentZoom(double zoom);

	void SwitchMode();
	void SetSelectionMode();
	void SetDrawingMode();
	bool IsSelectionMode() const {
		return mode == SELECTION_MODE;
	}
	bool IsDrawingMode() const {
		return mode == DRAWING_MODE;
	}

	// Brushes
	void FillDoodadPreviewBuffer();
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
	void SetSpawnTime(int time) {
		creature_spawntime = time;
	}

	void SetLightIntensity(float v) {
		light_intensity = v;
	}
	float GetLightIntensity() const {
		return light_intensity;
	}

	void SetAmbientLightLevel(float v) {
		ambient_light_level = v;
	}
	float GetAmbientLightLevel() const {
		return ambient_light_level;
	}
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

	// Load/unload a client version (takes care of dialogs aswell)

	void UnloadVersion() {
		g_version.UnloadVersion();
	}
	bool LoadVersion(ClientVersionID ver, wxString& error, wxArrayString& warnings, bool force = false) {
		return g_version.LoadVersion(ver, error, warnings, force);
	}
	// The current version loaded (returns CLIENT_VERSION_NONE if no version is loaded)
	const ClientVersion& GetCurrentVersion() const {
		return g_version.GetCurrentVersion();
	}
	ClientVersionID GetCurrentVersionID() const {
		return g_version.GetCurrentVersionID();
	}
	// If any version is loaded at all
	bool IsVersionLoaded() const {
		return g_version.IsVersionLoaded();
	}

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
		return aui_manager;
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

	// Map
	Map& GetCurrentMap();
	int GetOpenMapCount();
	bool ShouldSave();
	void SaveCurrentMap(FileName filename, bool showdialog); // "" means default filename
	void SaveCurrentMap(bool showdialog = true) {
		SaveCurrentMap(wxString(""), showdialog);
	}
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
	// Hidden from public view
	PaletteWindow* CreatePalette();

protected:
	//=========================================================================
	// Public members
	//=========================================================================
public:
	wxAuiManager* aui_manager;
	MapTabbook* tabbook;
	MainFrame* root; // The main frame
	WelcomeDialog* welcomeDialog;
	CopyBuffer copybuffer;

	MinimapWindow* minimap;
	DCButton* gem; // The small gem in the lower-right corner
	SearchResultWindow* search_result_window;
	GraphicManager gfx;

	BaseMap* secondary_map; // A buffer map
	BaseMap* doodad_buffer_map; // The map in which doodads are temporarily stored

	using PaletteList = std::list<PaletteWindow*>;
	PaletteList palettes;

	wxGLContext* OGLContext;

	EditorMode mode;

	bool pasting;

	Hotkey hotkeys[10];
	bool hotkeys_enabled;

	//=========================================================================
	// Brush references
	//=========================================================================

	HouseBrush* house_brush;
	HouseExitBrush* house_exit_brush;
	WaypointBrush* waypoint_brush;
	OptionalBorderBrush* optional_brush;
	EraserBrush* eraser;
	SpawnBrush* spawn_brush;
	DoorBrush* normal_door_brush;
	DoorBrush* locked_door_brush;
	DoorBrush* magic_door_brush;
	DoorBrush* quest_door_brush;
	DoorBrush* hatch_door_brush;
	DoorBrush* normal_door_alt_brush;
	DoorBrush* archway_door_brush;
	DoorBrush* window_door_brush;
	FlagBrush* pz_brush;
	FlagBrush* rook_brush;
	FlagBrush* nolog_brush;
	FlagBrush* pvp_brush;

	//=========================================================================
	// Internal brush data
	//=========================================================================
	Brush* current_brush;
	Brush* previous_brush;
	BrushShape brush_shape;
	int brush_size;
	int brush_variation;
	int creature_spawntime;

	bool draw_locked_doors;
	bool use_custom_thickness;
	float custom_thickness_mod;
	float light_intensity;
	float ambient_light_level;

protected:
	wxWindowDisabler* winDisabler;

	int disabled_counter;

	friend class RenderingLock;
	friend MapTab::MapTab(MapTabbook*, Editor*);
	friend MapTab::MapTab(const MapTab*);
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
