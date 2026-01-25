#include "editor/persistence/minimap_exporter.h"

#include "editor/editor.h"
#include "editor/persistence/editor_persistence.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"
#include <wx/string.h>
#include <format>

void MinimapExporter::exportMinimap(Editor& editor, const FileName& directory, const std::string& base_filename, ExportOption option, int specific_floor) {
	g_gui.CreateLoadBar("Exporting minimap");

	try {

		switch (option) {
			case EXPORT_ALL_FLOORS: { // All floors
				for (int floor = 0; floor < MAP_LAYERS; ++floor) {
					g_gui.SetLoadScale(int(floor * (100.f / 16.f)), int((floor + 1) * (100.f / 16.f)));
					FileName file(std::format("{}_{}.bmp", base_filename, floor));
					file.Normalize(wxPATH_NORM_ALL, directory.GetFullPath());

					EditorPersistence::exportMiniMap(editor, file, floor, true);
				}
				break;
			}

			case EXPORT_GROUND_FLOOR: { // Ground floor
				FileName file(std::format("{}_{}.bmp", base_filename, GROUND_LAYER));
				file.Normalize(wxPATH_NORM_ALL, directory.GetFullPath());

				EditorPersistence::exportMiniMap(editor, file, GROUND_LAYER, true);
				break;
			}

			case EXPORT_SPECIFIC_FLOOR: { // Specific floors
				FileName file(std::format("{}_{}.bmp", base_filename, specific_floor));
				file.Normalize(wxPATH_NORM_ALL, directory.GetFullPath());

				EditorPersistence::exportMiniMap(editor, file, specific_floor, true);
				break;
			}

			case EXPORT_SELECTED_AREA: { // Selected area
				EditorPersistence::exportSelectionAsMiniMap(editor, directory, base_filename);
				break;
			}
		}
	} catch (std::bad_alloc&) {
		DialogUtil::PopupDialog("Error", "There is not enough memory available to complete the operation.", wxOK);
	}

	g_gui.DestroyLoadBar();
}
