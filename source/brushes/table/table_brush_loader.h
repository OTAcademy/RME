//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_BRUSHES_TABLE_TABLE_BRUSH_LOADER_H
#define RME_BRUSHES_TABLE_TABLE_BRUSH_LOADER_H

#include "app/main.h"

class TableBrush;
class TableBrushItems;

class TableBrushLoader {
public:
	static bool load(pugi::xml_node node, TableBrush& brush, TableBrushItems& items, std::vector<std::string>& warnings);

private:
	static uint32_t getAlignment(const std::string& alignString, std::vector<std::string>& warnings);
};

#endif
