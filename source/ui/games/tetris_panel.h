#ifndef RME_UI_GAMES_TETRIS_PANEL_H
#define RME_UI_GAMES_TETRIS_PANEL_H

#include "ui/games/game_panel.h"

const int TETRIS_MAPHEIGHT = 20;
const int TETRIS_MAPWIDTH = 10;

class TetrisPanel : public GamePanel {
public:
	TetrisPanel(wxWindow* parent);
	~TetrisPanel();

protected:
	virtual void Render(wxDC& pdc);
	virtual void GameLoop(int time);
	virtual void OnKey(wxKeyEvent& event, bool down);

	virtual int getFPS() const {
		return lines / 10 + 3;
	}

	enum Color {
		NO_COLOR,
		RED,
		BLUE,
		GREEN,
		STEEL,
		YELLOW,
		PURPLE,
		WHITE,
	};

	enum BlockType {
		FIRST_BLOCK,
		BLOCK_TOWER = FIRST_BLOCK,
		BLOCK_SQUARE,
		BLOCK_TRIANGLE,
		BLOCK_L,
		BLOCK_J,
		BLOCK_Z,
		BLOCK_S,
		LAST_BLOCK = BLOCK_S
	};

	struct Block {
		Color structure[4][4];
		int x, y;
	} block;

	const wxBrush& GetBrush(Color color) const;
	bool BlockCollisionTest(int mx, int my) const;
	void RemoveRow(int row);
	void NewBlock();
	void MoveBlock(int x, int y);
	void RotateBlock();
	void NewGame();
	void EndGame();
	void AddScore(int lines);

	int score;
	int lines;
	Color map[TETRIS_MAPWIDTH][TETRIS_MAPHEIGHT];
};

#endif // RME_UI_GAMES_TETRIS_PANEL_H
