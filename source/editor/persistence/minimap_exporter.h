#ifndef RME_EDITOR_PERSISTENCE_MINIMAP_EXPORTER_H
#define RME_EDITOR_PERSISTENCE_MINIMAP_EXPORTER_H

#include "util/common.h"
#include <wx/string.h>

class Editor;

class MinimapExporter {
public:
	enum ExportOption {
		EXPORT_ALL_FLOORS = 0,
		EXPORT_GROUND_FLOOR = 1,
		EXPORT_SPECIFIC_FLOOR = 2,
		EXPORT_SELECTED_AREA = 3
	};

	static void exportMinimap(Editor& editor, const FileName& directory, const std::string& base_filename, ExportOption option, int specific_floor = 7);
};

#endif
