//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UI_MENUBAR_LOADER_H_
#define RME_UI_MENUBAR_LOADER_H_

#include "ui/main_menubar.h"
#include "ext/pugixml.hpp"
#include <unordered_map>
#include <list>
#include <string>

class MenuBarLoader {
public:
	static bool Load(const FileName& path, wxMenuBar* menubar, std::unordered_map<MenuBar::ActionID, std::list<wxMenuItem*>>& items, const std::unordered_map<std::string, std::unique_ptr<MenuBar::Action>>& actions, RecentFilesManager& recentFilesManager, std::vector<std::string>& warnings, wxString& error);

private:
	static wxObject* LoadItem(pugi::xml_node node, wxMenu* parent, std::unordered_map<MenuBar::ActionID, std::list<wxMenuItem*>>& items, const std::unordered_map<std::string, std::unique_ptr<MenuBar::Action>>& actions, RecentFilesManager& recentFilesManager, std::vector<std::string>& warnings, wxString& error);
};

#endif
