//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_MENUBAR_LOADER_H_
#define RME_UI_MENUBAR_LOADER_H_

#include "ui/main_menubar.h"
#include "ext/pugixml.hpp"
#include <map>
#include <list>
#include <string>

class MenuBarLoader {
public:
	static bool Load(const FileName& path, wxMenuBar* menubar, std::map<MenuBar::ActionID, std::list<wxMenuItem*>>& items, const std::map<std::string, MenuBar::Action*>& actions, RecentFilesManager& recentFilesManager, std::vector<std::string>& warnings, wxString& error);

private:
	static wxObject* LoadItem(pugi::xml_node node, wxMenu* parent, std::map<MenuBar::ActionID, std::list<wxMenuItem*>>& items, const std::map<std::string, MenuBar::Action*>& actions, RecentFilesManager& recentFilesManager, std::vector<std::string>& warnings, wxString& error);
};

#endif
