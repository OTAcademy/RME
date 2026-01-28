#include "app/main.h"
#include "ui/games/tetris_panel.h"
#include "ui/dialog_util.h"
#include "util/mt_rand.h"
#include <memory>

//=============================================================================
// TetrisPanel - A window with a Tetris game!

TetrisPanel::TetrisPanel(wxWindow* parent) :
	GamePanel(parent, 16 * TETRIS_MAPWIDTH, 16 * TETRIS_MAPHEIGHT) {
	NewGame();
}

TetrisPanel::~TetrisPanel() {
	////
}

const wxBrush& TetrisPanel::GetBrush(Color color) const {
	static std::unique_ptr<wxBrush> yellow_brush;
	static std::unique_ptr<wxBrush> purple_brush;

	if (yellow_brush.get() == nullptr) {
		yellow_brush.reset(newd wxBrush(wxColor(255, 255, 0)));
	}
	if (purple_brush.get() == nullptr) {
		purple_brush.reset(newd wxBrush(wxColor(128, 0, 255)));
	}

	const wxBrush* brush = nullptr;
	switch (color) {
		case RED:
			brush = wxRED_BRUSH;
			break;
		case BLUE:
			brush = wxCYAN_BRUSH;
			break;
		case GREEN:
			brush = wxGREEN_BRUSH;
			break;
		case PURPLE:
			brush = purple_brush.get();
			break;
		case YELLOW:
			brush = yellow_brush.get();
			break;
		case WHITE:
			brush = wxWHITE_BRUSH;
			break;
		case STEEL:
			brush = wxGREY_BRUSH;
			break;
		default:
			brush = wxBLACK_BRUSH;
			break;
	}
	return *brush;
}

void TetrisPanel::Render(wxDC& pdc) {
	pdc.SetBackground(*wxBLACK_BRUSH);
	pdc.Clear();

	for (int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
		for (int x = 0; x < TETRIS_MAPWIDTH; ++x) {
			pdc.SetBrush(GetBrush(map[x][y]));
			pdc.DrawRectangle(x * 16, y * 16, 16, 16);
		}
	}

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			if (block.structure[x][y] != NO_COLOR) {
				pdc.SetBrush(GetBrush(block.structure[x][y]));
				pdc.DrawRectangle((block.x + x) * 16, (block.y + y) * 16, 16, 16);
			}
		}
	}
}

void TetrisPanel::OnKey(wxKeyEvent& event, bool down) {
	if (!down || dead) {
		return;
	}

	switch (event.GetKeyCode()) {
		case WXK_SPACE: {
			if (paused()) {
				unpause();
			} else {
				pause();
			}
			break;
		}
		case WXK_NUMPAD_UP:
		case WXK_UP: {
			if (dead) {
				return;
			}
			unpause();
			RotateBlock();
			break;
		}
		case WXK_NUMPAD_DOWN:
		case WXK_DOWN: {
			if (dead) {
				return;
			}
			unpause();
			MoveBlock(0, 1);
			break;
		}
		case WXK_NUMPAD_LEFT:
		case WXK_LEFT: {
			if (dead) {
				return;
			}
			unpause();
			MoveBlock(-1, 0);
			break;
		}
		case WXK_NUMPAD_RIGHT:
		case WXK_RIGHT: {
			if (dead) {
				return;
			}
			unpause();
			MoveBlock(1, 0);
			break;
		}
	}
}

void TetrisPanel::NewGame() {
	NewBlock();
	score = 0;
	lines = 0;
	game_timer.Start();
	unpause();
	dead = false;

	// Clear map
	for (int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
		for (int x = 0; x < TETRIS_MAPWIDTH; ++x) {
			map[x][y] = NO_COLOR;
		}
	}
	AddScore(0); // Update title
}

void TetrisPanel::AddScore(int lines_added) {
	lines += lines_added;
	score += lines_added * lines_added * 10;
	wxString title = "Remere's Tetris : ";
	title << score << " points  ";
	title << lines << " lines";
	((wxTopLevelWindow*)GetParent())->SetTitle(title);
}

void TetrisPanel::GameLoop(int time) {
	MoveBlock(0, 1);
}

bool TetrisPanel::BlockCollisionTest(int mx, int my) const {
	int nx = block.x + mx;
	int ny = block.y + my;

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			if (block.structure[x][y] != NO_COLOR) {
				if (nx + x < 0 || nx + x > TETRIS_MAPWIDTH - 1 || ny + y < 0 || ny + y > TETRIS_MAPHEIGHT - 1) {
					return true;
				}
			}
		}
	}

	for (int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
		for (int x = 0; x < TETRIS_MAPWIDTH; ++x) {
			if (x >= nx && x < nx + 4 && y >= ny && y < ny + 4) {
				if (map[x][y] != NO_COLOR && block.structure[x - nx][y - ny] != NO_COLOR) {
					return true;
				}
			}
		}
	}
	return false;
}

void TetrisPanel::RemoveRow(int row) {
	for (int x = 0; x < TETRIS_MAPWIDTH; ++x) {
		for (int y = row; y > 0; --y) { // Move all above one step down
			map[x][y] = map[x][y - 1];
		}
	}
}

void TetrisPanel::NewBlock() {
	block.x = TETRIS_MAPWIDTH / 2;
	block.y = -1;

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			block.structure[x][y] = NO_COLOR;
		}
	}

	switch (random(FIRST_BLOCK, LAST_BLOCK)) {
		case BLOCK_TOWER: {
			block.structure[1][0] = RED;
			block.structure[1][1] = RED;
			block.structure[1][2] = RED;
			block.structure[1][3] = RED;
			block.y = 0;
			break;
		}
		default:
		case BLOCK_SQUARE: {
			block.structure[1][1] = BLUE;
			block.structure[1][2] = BLUE;
			block.structure[2][1] = BLUE;
			block.structure[2][2] = BLUE;
			break;
		}
		case BLOCK_TRIANGLE: {
			block.structure[1][1] = STEEL;
			block.structure[0][2] = STEEL;
			block.structure[1][2] = STEEL;
			block.structure[2][2] = STEEL;
			break;
		}
		case BLOCK_Z: {
			block.structure[0][1] = YELLOW;
			block.structure[1][1] = YELLOW;
			block.structure[1][2] = YELLOW;
			block.structure[2][2] = YELLOW;
			break;
		}
		case BLOCK_S: {
			block.structure[2][1] = GREEN;
			block.structure[1][1] = GREEN;
			block.structure[1][2] = GREEN;
			block.structure[0][2] = GREEN;
			break;
		}
		case BLOCK_J: {
			block.structure[1][1] = WHITE;
			block.structure[2][1] = WHITE;
			block.structure[2][2] = WHITE;
			block.structure[2][3] = WHITE;
			break;
		}
		case BLOCK_L: {
			block.structure[2][1] = PURPLE;
			block.structure[1][1] = PURPLE;
			block.structure[1][2] = PURPLE;
			block.structure[1][3] = PURPLE;
			// break; missing add it?
		}
	}
}

void TetrisPanel::MoveBlock(int x, int y) {
	if (BlockCollisionTest(x, y)) {
		if (y == 1) { // moving down...
			if (block.y < 1) { // Out of bounds!
				dead = true;
				DialogUtil::PopupDialog("Game Over", "You reached a score of " + i2ws(score) + "!", wxOK);
				NewGame();
				SetFocus();
			} else {
				// Freeze the old block onto the map
				for (int y = 0; y < 4; ++y) {
					for (int x = 0; x < 4; ++x) {
						if (block.structure[x][y] != NO_COLOR) {
							map[block.x + x][block.y + y] = block.structure[x][y];
						}
					}
				}

				// Any cleared rows?
				int cleared = 0;
				for (int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
					bool full = true;
					for (int x = 0; x < TETRIS_MAPWIDTH; ++x) {
						if (map[x][y] == NO_COLOR) {
							full = false;
						}
					}
					if (full) {
						RemoveRow(y);
						++cleared;
					}
				}
				AddScore(cleared);
				NewBlock();
			}
		} // If we're not moving down, we're not moving the block either
	} else {
		// No collision so move the block!
		block.x += x;
		block.y += y;
	}

	Refresh();
}

void TetrisPanel::RotateBlock() {
	Block temp;

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			temp.structure[3 - x][y] = block.structure[y][x];
		}
	}

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			if (temp.structure[x][y] != NO_COLOR) {
				if (block.x + x < 0 || block.x + x > TETRIS_MAPWIDTH - 1 || block.y + y < 0 || block.y + y > TETRIS_MAPHEIGHT - 1) {
					return;
				}
			}
		}
	}

	for (int y = 0; y < TETRIS_MAPWIDTH; ++y) {
		for (int x = 0; x < TETRIS_MAPHEIGHT; ++x) {
			if (x >= block.x && x < block.x + 4 && y >= block.y && y < block.y + 4) {
				if (map[x][y] != NO_COLOR && temp.structure[x - block.x][y - block.y] != NO_COLOR) {
					return;
				}
			}
		}
	}

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			block.structure[x][y] = temp.structure[x][y];
		}
	}

	Refresh();
}
