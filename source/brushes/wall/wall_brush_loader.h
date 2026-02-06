//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_WALL_BRUSH_LOADER_H
#define RME_WALL_BRUSH_LOADER_H

#include <vector>
#include <string>

// Forward declarations
namespace pugi {
	class xml_node;
}
// class std::vector<std::string>;
class WallBrush;
class WallBrushItems;

class WallBrushLoader {
public:
	static bool load(WallBrush* brush, WallBrushItems& items, pugi::xml_node node, std::vector<std::string>& warnings);
};

#endif
