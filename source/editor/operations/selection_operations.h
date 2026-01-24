#ifndef RME_SELECTION_OPERATIONS_H
#define RME_SELECTION_OPERATIONS_H

#include "map/position.h"
#include <list>

class Editor;
class Tile;
class DoodadBrush;

class SelectionOperations {
public:
	static void moveSelection(Editor& editor, Position offset);
	static void destroySelection(Editor& editor);
	static void borderizeSelection(Editor& editor);
	static void randomizeSelection(Editor& editor);

	// Helper functions used by Editor::drawInternal
	static void removeDuplicateWalls(Tile* buffer_tile, Tile* new_tile);
	static void doSurroundingBorders(DoodadBrush* doodad_brush, std::list<Position>& tilestoborder, Tile* buffer_tile, Tile* new_tile);
};

#endif
