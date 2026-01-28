#ifndef RME_UI_GAMES_SNAKE_PANEL_H
#define RME_UI_GAMES_SNAKE_PANEL_H

#include "ui/games/game_panel.h"

const int SNAKE_MAPHEIGHT = 20;
const int SNAKE_MAPWIDTH = 20;

class SnakePanel : public GamePanel {
public:
	SnakePanel(wxWindow* parent);
	~SnakePanel();

protected:
	virtual void Render(wxDC& pdc);
	virtual void GameLoop(int time);
	virtual void OnKey(wxKeyEvent& event, bool down);

	virtual int getFPS() const {
		return 7;
	}

	enum {
		NORTH,
		SOUTH,
		WEST,
		EAST,
	};

	void NewApple();
	void Move(int dir);
	void NewGame();
	void EndGame();
	void UpdateTitle();

	// -1 is apple, 0 is nothing, >0 is snake (will decay in n rounds)
	int length;
	int last_dir;
	int map[SNAKE_MAPWIDTH][SNAKE_MAPHEIGHT];
};

#endif // RME_UI_GAMES_SNAKE_PANEL_H
