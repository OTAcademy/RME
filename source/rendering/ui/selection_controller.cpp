//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "rendering/ui/selection_controller.h"
#include "rendering/ui/map_display.h"
#include "editor.h"
#include "map.h"
#include "tile.h"
#include "item.h"
#include "spawn.h"
#include "creature.h"
#include "settings.h"
#include "gui.h"
#include "rendering/ui/brush_selector.h"
#include "creature_brush.h"
#include "raw_brush.h"

SelectionController::SelectionController(MapCanvas* canvas, Editor& editor) :
	canvas(canvas),
	editor(editor),
	dragging(false),
	boundbox_selection(false),
	drag_start_pos(Position()) {
}

SelectionController::~SelectionController() {
}

void SelectionController::HandleClick(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down) {
	if (ctrl_down && alt_down) {
		Tile* tile = editor.map.getTile(mouse_map_pos);
		if (tile && tile->size() > 0) {
			// Select visible creature
			if (tile->creature && g_settings.getInteger(Config::SHOW_CREATURES)) {
				CreatureBrush* brush = tile->creature->getBrush();
				if (brush) {
					g_gui.SelectBrush(brush, TILESET_CREATURE);
					return;
				}
			}
			// Fall back to item selection
			Item* item = tile->getTopItem();
			if (item && item->getRAWBrush()) {
				g_gui.SelectBrush(item->getRAWBrush(), TILESET_RAW);
			}
		}
	} else if (g_gui.IsSelectionMode()) {
		if (canvas->isPasting()) {
			// Set paste to false (no rendering etc.)
			canvas->EndPasting();

			// Paste to the map
			editor.copybuffer.paste(editor, mouse_map_pos);

			// Start dragging
			dragging = true;
			drag_start_pos = mouse_map_pos;
		} else {
			boundbox_selection = false;
			if (shift_down) {
				boundbox_selection = true;

				if (!ctrl_down) {
					editor.selection.start(); // Start selection session
					editor.selection.clear(); // Clear out selection
					editor.selection.finish(); // End selection session
					editor.selection.updateSelectionCount();
				}
			} else if (ctrl_down) {
				Tile* tile = editor.map.getTile(mouse_map_pos);
				if (tile) {
					if (tile->spawn && g_settings.getInteger(Config::SHOW_SPAWNS)) {
						editor.selection.start(); // Start selection session
						if (tile->spawn->isSelected()) {
							editor.selection.remove(tile, tile->spawn);
						} else {
							editor.selection.add(tile, tile->spawn);
						}
						editor.selection.finish(); // Finish selection session
						editor.selection.updateSelectionCount();
					} else if (tile->creature && g_settings.getInteger(Config::SHOW_CREATURES)) {
						editor.selection.start(); // Start selection session
						if (tile->creature->isSelected()) {
							editor.selection.remove(tile, tile->creature);
						} else {
							editor.selection.add(tile, tile->creature);
						}
						editor.selection.finish(); // Finish selection session
						editor.selection.updateSelectionCount();
					} else {
						Item* item = tile->getTopItem();
						if (item) {
							editor.selection.start(); // Start selection session
							if (item->isSelected()) {
								editor.selection.remove(tile, item);
							} else {
								editor.selection.add(tile, item);
							}
							editor.selection.finish(); // Finish selection session
							editor.selection.updateSelectionCount();
						}
					}
				}
			} else {
				Tile* tile = editor.map.getTile(mouse_map_pos);
				if (!tile) {
					editor.selection.start(); // Start selection session
					editor.selection.clear(); // Clear out selection
					editor.selection.finish(); // End selection session
					editor.selection.updateSelectionCount();
				} else if (tile->isSelected()) {
					dragging = true;
					drag_start_pos = mouse_map_pos;
				} else {
					editor.selection.start(); // Start a selection session
					editor.selection.clear();
					editor.selection.commit();
					if (tile->spawn && g_settings.getInteger(Config::SHOW_SPAWNS)) {
						editor.selection.add(tile, tile->spawn);
						dragging = true;
						drag_start_pos = mouse_map_pos;
					} else if (tile->creature && g_settings.getInteger(Config::SHOW_CREATURES)) {
						editor.selection.add(tile, tile->creature);
						dragging = true;
						drag_start_pos = mouse_map_pos;
					} else {
						Item* item = tile->getTopItem();
						if (item) {
							editor.selection.add(tile, item);
							dragging = true;
							drag_start_pos = mouse_map_pos;
						}
					}
					editor.selection.finish(); // Finish the selection session
					editor.selection.updateSelectionCount();
				}
			}
		}
	}
}

void SelectionController::HandleDrag(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down) {
	if (g_gui.IsSelectionMode()) {
		if (canvas->isPasting()) {
			canvas->Refresh();
		} else if (dragging) {
			wxString ss;

			int move_x = drag_start_pos.x - mouse_map_pos.x;
			int move_y = drag_start_pos.y - mouse_map_pos.y;
			int move_z = drag_start_pos.z - mouse_map_pos.z;
			ss << "Dragging " << -move_x << "," << -move_y << "," << -move_z;
			g_gui.SetStatusText(ss);

			canvas->Refresh();
		} else if (boundbox_selection) {
			// Calculate selection size
			int move_x = std::abs(canvas->last_click_map_x - mouse_map_pos.x);
			int move_y = std::abs(canvas->last_click_map_y - mouse_map_pos.y);
			wxString ss;
			ss << "Selection " << move_x + 1 << ":" << move_y + 1;
			g_gui.SetStatusText(ss);

			canvas->Refresh();
		}
	}
}

void SelectionController::HandleRelease(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down) {
	int move_x = canvas->last_click_map_x - mouse_map_pos.x;
	int move_y = canvas->last_click_map_y - mouse_map_pos.y;
	int move_z = canvas->last_click_map_z - mouse_map_pos.z;

	if (g_gui.IsSelectionMode()) {
		if (dragging && (move_x != 0 || move_y != 0 || move_z != 0)) {
			editor.moveSelection(Position(move_x, move_y, move_z));
		} else {
			if (boundbox_selection) {
				if (mouse_map_pos.x == canvas->last_click_map_x && mouse_map_pos.y == canvas->last_click_map_y && ctrl_down) {
					// Mouse hasn't moved, do control+shift thingy!
					Tile* tile = editor.map.getTile(mouse_map_pos);
					if (tile) {
						editor.selection.start(); // Start a selection session
						if (tile->isSelected()) {
							editor.selection.remove(tile);
						} else {
							editor.selection.add(tile);
						}
						editor.selection.finish(); // Finish the selection session
						editor.selection.updateSelectionCount();
					}
				} else {
					ExecuteBoundboxSelection(Position(canvas->last_click_map_x, canvas->last_click_map_y, canvas->last_click_map_z), mouse_map_pos, mouse_map_pos.z);
				}
			} else if (ctrl_down) {
				////
			} else {
				// User hasn't moved anything, meaning selection/deselection
				Tile* tile = editor.map.getTile(mouse_map_pos);
				if (tile) {
					if (tile->spawn && g_settings.getInteger(Config::SHOW_SPAWNS)) {
						if (!tile->spawn->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, tile->spawn);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					} else if (tile->creature && g_settings.getInteger(Config::SHOW_CREATURES)) {
						if (!tile->creature->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, tile->creature);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					} else {
						Item* item = tile->getTopItem();
						if (item && !item->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, item);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					}
				}
			}
		}
		editor.actionQueue->resetTimer();
		dragging = false;
		boundbox_selection = false;
	}
}

void SelectionController::HandlePropertiesClick(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down) {
	Tile* tile = editor.map.getTile(mouse_map_pos);

	if (g_gui.IsDrawingMode()) {
		g_gui.SetSelectionMode();
	}

	canvas->EndPasting();

	boundbox_selection = false;
	if (shift_down) {
		boundbox_selection = true;

		if (!ctrl_down) {
			editor.selection.start(); // Start selection session
			editor.selection.clear(); // Clear out selection
			editor.selection.finish(); // End selection session
			editor.selection.updateSelectionCount();
		}
	} else if (!tile) {
		editor.selection.start(); // Start selection session
		editor.selection.clear(); // Clear out selection
		editor.selection.finish(); // End selection session
		editor.selection.updateSelectionCount();
	} else if (tile->isSelected()) {
		// Do nothing!
	} else {
		editor.selection.start(); // Start a selection session
		editor.selection.clear();
		editor.selection.commit();
		if (tile->spawn && g_settings.getInteger(Config::SHOW_SPAWNS)) {
			editor.selection.add(tile, tile->spawn);
		} else if (tile->creature && g_settings.getInteger(Config::SHOW_CREATURES)) {
			editor.selection.add(tile, tile->creature);
		} else {
			Item* item = tile->getTopItem();
			if (item) {
				editor.selection.add(tile, item);
			}
		}
		editor.selection.finish(); // Finish the selection session
		editor.selection.updateSelectionCount();
	}
}

void SelectionController::HandlePropertiesRelease(const Position& mouse_map_pos, bool shift_down, bool ctrl_down, bool alt_down) {
	if (g_gui.IsDrawingMode()) {
		g_gui.SetSelectionMode();
	}

	if (boundbox_selection) {
		if (mouse_map_pos.x == canvas->last_click_map_x && mouse_map_pos.y == canvas->last_click_map_y && ctrl_down) {
			// Mouse hasn't move, do control+shift thingy!
			Tile* tile = editor.map.getTile(mouse_map_pos);
			if (tile) {
				editor.selection.start(); // Start a selection session
				if (tile->isSelected()) {
					editor.selection.remove(tile);
				} else {
					editor.selection.add(tile);
				}
				editor.selection.finish(); // Finish the selection session
				editor.selection.updateSelectionCount();
			}
		} else {
			ExecuteBoundboxSelection(Position(canvas->last_click_map_x, canvas->last_click_map_y, canvas->last_click_map_z), mouse_map_pos, mouse_map_pos.z);
		}
	} else if (ctrl_down) {
		// Nothing
	}

	editor.actionQueue->resetTimer();
	dragging = false;
	boundbox_selection = false;
}

void SelectionController::ExecuteBoundboxSelection(const Position& start_pos, const Position& end_pos, int floor) {
	int start_x = start_pos.x;
	int start_y = start_pos.y;
	int end_x = end_pos.x;
	int end_y = end_pos.y;

	if (start_x > end_x) {
		std::swap(start_x, end_x);
	}
	if (start_y > end_y) {
		std::swap(start_y, end_y);
	}

	int numtiles = 0;
	int threadcount = std::max(g_settings.getInteger(Config::WORKER_THREADS), 1);

	int s_x = 0, s_y = 0, s_z = 0;
	int e_x = 0, e_y = 0, e_z = 0;

	switch (g_settings.getInteger(Config::SELECTION_TYPE)) {
		case SELECT_CURRENT_FLOOR: {
			s_z = e_z = floor;
			s_x = start_x;
			s_y = start_y;
			e_x = end_x;
			e_y = end_y;
			break;
		}
		case SELECT_ALL_FLOORS: {
			s_x = start_x;
			s_y = start_y;
			s_z = MAP_MAX_LAYER;
			e_x = end_x;
			e_y = end_y;
			e_z = floor;

			if (g_settings.getInteger(Config::COMPENSATED_SELECT)) {
				s_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
				s_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);

				e_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
				e_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
			}

			numtiles = (s_z - e_z) * (e_x - s_x) * (e_y - s_y);
			break;
		}
		case SELECT_VISIBLE_FLOORS: {
			s_x = start_x;
			s_y = start_y;
			if (floor <= GROUND_LAYER) {
				s_z = GROUND_LAYER;
			} else {
				s_z = std::min(MAP_MAX_LAYER, floor + 2);
			}
			e_x = end_x;
			e_y = end_y;
			e_z = floor;

			if (g_settings.getInteger(Config::COMPENSATED_SELECT)) {
				s_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
				s_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);

				e_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
				e_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
			}
			break;
		}
	}

	if (numtiles < 500) {
		// No point in threading for such a small set.
		threadcount = 1;
	}
	// Subdivide the selection area
	// We know it's a square, just split it into several areas
	int width = e_x - s_x;
	if (width < threadcount) {
		threadcount = min(1, width);
	}
	// Let's divide!
	int remainder = width;
	int cleared = 0;
	std::vector<SelectionThread*> threads;
	if (width == 0) {
		threads.push_back(newd SelectionThread(editor, Position(s_x, s_y, s_z), Position(s_x, e_y, e_z)));
	} else {
		for (int i = 0; i < threadcount; ++i) {
			int chunksize = width / threadcount;
			// The last threads takes all the remainder
			if (i == threadcount - 1) {
				chunksize = remainder;
			}
			threads.push_back(newd SelectionThread(editor, Position(s_x + cleared, s_y, s_z), Position(s_x + cleared + chunksize, e_y, e_z)));
			cleared += chunksize;
			remainder -= chunksize;
		}
	}
	ASSERT(cleared == width);
	ASSERT(remainder == 0);

	editor.selection.start(); // Start a selection session
	for (std::vector<SelectionThread*>::iterator iter = threads.begin(); iter != threads.end(); ++iter) {
		(*iter)->Execute();
	}
	for (std::vector<SelectionThread*>::iterator iter = threads.begin(); iter != threads.end(); ++iter) {
		editor.selection.join(*iter);
	}
	editor.selection.finish(); // Finish the selection session
	editor.selection.updateSelectionCount();
}
