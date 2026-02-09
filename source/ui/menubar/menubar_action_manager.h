#ifndef RME_UI_MENUBAR_MENUBAR_ACTION_MANAGER_H_
#define RME_UI_MENUBAR_MENUBAR_ACTION_MANAGER_H_

#include <unordered_map>
#include <string>
#include <memory>

// Forward declarations
class MainMenuBar;
namespace MenuBar {
	struct Action;
}

class MenuBarActionManager {
public:
	static void RegisterActions(MainMenuBar* mb, std::unordered_map<std::string, std::unique_ptr<MenuBar::Action>>& actions);
	static void UpdateState(MainMenuBar* mb);
};

#endif
