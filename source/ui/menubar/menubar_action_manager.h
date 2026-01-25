#ifndef RME_UI_MENUBAR_MENUBAR_ACTION_MANAGER_H_
#define RME_UI_MENUBAR_MENUBAR_ACTION_MANAGER_H_

#include <map>
#include <string>

// Forward declarations
class MainMenuBar;
namespace MenuBar {
	struct Action;
}

class MenuBarActionManager {
public:
	static void RegisterActions(MainMenuBar* mb, std::map<std::string, MenuBar::Action*>& actions);
	static void UpdateState(MainMenuBar* mb);
};

#endif
