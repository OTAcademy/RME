#ifndef RME_UI_MENUBAR_FILE_MENU_HANDLER_H
#define RME_UI_MENUBAR_FILE_MENU_HANDLER_H

#include <wx/wx.h>

class MainFrame;
class MainMenuBar;

class FileMenuHandler : public wxEvtHandler {
public:
	FileMenuHandler(MainFrame* frame, MainMenuBar* menubar);
	~FileMenuHandler();

	void OnNew(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveAs(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);
	void OnGenerateMap(wxCommandEvent& event);

	void OnImportMap(wxCommandEvent& event);
	void OnImportMonsterData(wxCommandEvent& event);
	void OnImportMinimap(wxCommandEvent& event);

	void OnExportMinimap(wxCommandEvent& event);
	void OnExportTilesets(wxCommandEvent& event);

	void OnReloadDataFiles(wxCommandEvent& event);
	void OnPreferences(wxCommandEvent& event);
	void OnListExtensions(wxCommandEvent& event);
	void OnGotoWebsite(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

protected:
	MainFrame* frame;
	MainMenuBar* menubar;
};

#endif
