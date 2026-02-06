//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_CARPET_BRUSH_LOADER_H
#define RME_CARPET_BRUSH_LOADER_H

#include <wx/string.h>
#include <wx/arrstr.h>
#include "app/rme_forward_declarations.h"

// Forward declarations
namespace pugi {
	class xml_node;
}
class CarpetBrush;

class CarpetBrushLoader {
public:
	static bool load(CarpetBrush& brush, pugi::xml_node node, std::vector<std::string>& warnings);
};

#endif
