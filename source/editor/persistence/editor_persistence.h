#ifndef RME_EDITOR_PERSISTENCE_H
#define RME_EDITOR_PERSISTENCE_H

#include "util/common.h"
#include "app/rme_forward_declarations.h"
#include "io/iomap.h"
#include <wx/string.h>

class Editor;

class EditorPersistence {
public:
	static void saveMap(Editor& editor, FileName filename, bool showdialog);
	static void loadMap(Editor& editor, const FileName& filename);
	static bool importMap(Editor& editor, FileName filename, int import_x_offset, int import_y_offset, ImportType house_import_type, ImportType spawn_import_type);
	static bool importMiniMap(Editor& editor, FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset);
};

#endif
