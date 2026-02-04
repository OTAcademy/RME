//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_MAIN_FRAME_H_
#define RME_UI_MAIN_FRAME_H_

#include "app/main.h"

class MainMenuBar;
class MainToolBar;

class MainFrame : public wxFrame {
public:
	MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~MainFrame() override;

	void UpdateMenubar();
	bool DoQueryClose();
	bool DoQuerySave(bool doclose = true);
	bool DoQuerySaveTileset(bool doclose = true);
	bool DoQueryImportCreatures();
	bool LoadMap(FileName name);

	void AddRecentFile(const FileName& file);
	void LoadRecentFiles();
	void SaveRecentFiles();
	std::vector<wxString> GetRecentFiles();

	MainToolBar* GetAuiToolBar() const {
		return tool_bar.get();
	}

	MainMenuBar* GetMainMenuBar() const {
		return menu_bar.get();
	}

	void OnUpdateMenus(wxCommandEvent& event);
	void UpdateFloorMenu();
	void OnIdle(wxIdleEvent& event);
	void OnExit(wxCloseEvent& event);

#ifdef _USE_UPDATER_
	void OnUpdateReceived(wxCommandEvent& event);
#endif

#ifdef __WINDOWS__
	virtual bool MSWTranslateMessage(WXMSG* msg);
#endif

	void PrepareDC(wxDC& dc);

protected:
	std::unique_ptr<MainMenuBar> menu_bar;
	std::unique_ptr<MainToolBar> tool_bar;

	friend class Application;
	friend class GUI;
};

#endif
