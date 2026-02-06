//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DOODAD_BRUSH_LOADER_H
#define RME_DOODAD_BRUSH_LOADER_H

#include "brushes/doodad/doodad_brush_types.h"
#include "brushes/doodad/doodad_brush_items.h"
#include <wx/string.h>
#include <wx/arrstr.h>

// Forward declarations
namespace pugi {
	class xml_node;
}
class DoodadBrush; // Optional, if we need it for assignment

class DoodadBrushLoader {
public:
	static bool load(pugi::xml_node node, DoodadBrushItems& items, DoodadBrushSettings& settings, std::vector<std::string>& warnings, DoodadBrush* brushPtr);

private:
	static bool loadAlternative(pugi::xml_node node, DoodadBrushItems& items, std::vector<std::string>& warnings, DoodadBrushItems::AlternativeBlock* which, DoodadBrush* brushPtr);
};

#endif
