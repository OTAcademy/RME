#include "app/main.h"
#include "ui/games/snake_panel.h"
#include "ui/dialog_util.h"
#include "util/mt_rand.h"

//=============================================================================
// SnakePanel - A window with a Snake game!

SnakePanel::SnakePanel(wxWindow* parent) :
	GamePanel(parent, 16 * SNAKE_MAPWIDTH, 16 * SNAKE_MAPHEIGHT) {
	NewGame();
}

SnakePanel::~SnakePanel() {
	////
}

void SnakePanel::Render(wxDC& pdc) {
	pdc.SetBackground(*wxBLACK_BRUSH);
	pdc.Clear();

	wxBrush snakebrush(wxColor(0, 0, 255));
	wxBrush applebrush(wxColor(255, 0, 0));

	double lblue = 1.0;
	double lred = 0.5;
	double lgreen = 0.0;

	for (int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for (int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			if (map[x][y] == -1) { // Apple
				pdc.SetBrush(applebrush);
				pdc.DrawRectangle(x * 16, y * 16, 16, 16);
			} else if (map[x][y] > 0) { // Snake
				double snook = double(map[x][y]) / length;
				snakebrush.SetColour(wxColor(
					int(255.0 * (1.0 - abs(lred - snook))),
					int(255.0 * (1.0 - abs(lgreen - snook))),
					int(255.0 * (1.0 - abs(lblue - snook)))
				));
				pdc.SetBrush(snakebrush);
				pdc.DrawRectangle(x * 16, y * 16, 16, 16);
			}
		}
	}
}

void SnakePanel::OnKey(wxKeyEvent& event, bool down) {
	if (!down) {
		return;
	}

	int keyCode = event.GetKeyCode();
	if (keyCode == WXK_SPACE) {
		if (paused()) {
			unpause();
		} else {
			pause();
		}
	} else if (!dead) {
		switch (keyCode) {
			case WXK_NUMPAD_UP:
			case WXK_UP: {
				unpause();
				Move(NORTH);
				break;
			}
			case WXK_NUMPAD_DOWN:
			case WXK_DOWN: {
				unpause();
				Move(SOUTH);
				break;
			}
			case WXK_NUMPAD_LEFT:
			case WXK_LEFT: {
				unpause();
				Move(WEST);
				break;
			}
			case WXK_NUMPAD_RIGHT:
			case WXK_RIGHT: {
				unpause();
				Move(EAST);
				break;
			}
		}
	}
}

void SnakePanel::NewGame() {
	length = 3;
	game_timer.Start();
	last_dir = NORTH;
	unpause();
	dead = false;

	// Clear map
	for (int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for (int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			map[x][y] = 0;
		}
	}
	map[SNAKE_MAPWIDTH / 2][SNAKE_MAPHEIGHT / 2] = length;
	NewApple();
	UpdateTitle(); // Update title
}

void SnakePanel::UpdateTitle() {
	wxString title = "Remere's Snake : ";
	title << length << " segments";
	((wxTopLevelWindow*)GetParent())->SetTitle(title);
}

void SnakePanel::GameLoop(int time) {
	Move(last_dir);
}

void SnakePanel::NewApple() {
	bool possible = false;
	for (int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for (int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			if (map[x][y] == 0) {
				possible = true;
			}
			if (possible) {
				break;
			}
		}
		if (possible) {
			break;
		}
	}

	if (possible) {
		while (true) {
			int x = random(0, SNAKE_MAPWIDTH - 1);
			int y = random(0, SNAKE_MAPHEIGHT - 1);
			if (map[x][y] == 0) {
				map[x][y] = -1;
				break;
			}
		}
	}
}

void SnakePanel::Move(int dir) {
	if ((last_dir == NORTH && dir == SOUTH) || (last_dir == WEST && dir == EAST) || (last_dir == EAST && dir == WEST) || (last_dir == SOUTH && dir == NORTH)) {
		return;
	}

	int nx = 0, ny = 0;
	int head_x = 0, head_y = 0;
	for (int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for (int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			if (map[x][y] == length) {
				head_x = x;
				head_y = y;
			}
		}
	}
	switch (dir) {
		case NORTH: {
			nx = head_x;
			ny = head_y - 1;
			break;
		}
		case SOUTH: {
			nx = head_x;
			ny = head_y + 1;
			break;
		}
		case WEST: {
			nx = head_x - 1;
			ny = head_y;
			break;
		}
		case EAST: {
			nx = head_x + 1;
			ny = head_y;
			break;
		}
		default:
			return;
	}

	if (map[nx][ny] > 0 || nx < 0 || ny < 0 || nx >= SNAKE_MAPWIDTH || ny >= SNAKE_MAPHEIGHT) {
		// Crash
		dead = true;
		DialogUtil::PopupDialog("Game Over", "You reached a length of " + i2ws(length) + "!", wxOK);
		NewGame();
		SetFocus();
	} else {
		// Walk!
		if (map[nx][ny] == -1) {
			// Took apple!
			length += 1;
			UpdateTitle();
			NewApple();
		} else {
			for (int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
				for (int x = 0; x < SNAKE_MAPWIDTH; ++x) {
					if (map[x][y] > 0) {
						map[x][y] -= 1;
					}
				}
			}
		}
		map[nx][ny] = length;
	}
	last_dir = dir;

	Refresh();
}
