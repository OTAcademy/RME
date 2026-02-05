#include "editor/persistence/tileset_exporter.h"

#include "game/materials.h"
#include "game/items.h"
#include "brushes/brush.h"
#include "brushes/raw/raw_brush.h"
#include "ui/gui.h"
#include "ui/dialog_util.h"
#include "ui/controls/sortable_list_box.h"
#include "app/application.h" // For g_settings
#include "ext/pugixml.hpp"

void TilesetExporter::exportTilesets(const FileName& directory, const std::string& filename) {
	g_gui.CreateLoadBar("Exporting Tilesets");

	try {

		FileName file(filename + ".xml");
		file.Normalize(wxPATH_NORM_ALL, directory.GetFullPath());

		pugi::xml_document doc;
		pugi::xml_node node = doc.append_child("materials");

		std::map<std::string, TilesetCategoryType> palettes {
			{ "Terrain", TILESET_TERRAIN },
			{ "Doodad", TILESET_DOODAD },
			{ "Items", TILESET_ITEM },
			{ "Collection", TILESET_COLLECTION },
			{ "Raw", TILESET_RAW }
		};
		for (auto& [iter_name, tileset_ptr] : g_materials.tilesets) {
			std::string _data = tileset_ptr->name;
			std::transform(_data.begin(), _data.end(), _data.begin(), [](unsigned char c) { return std::tolower(c); });
			if (_data == "others") {
				bool blocked = 1;

				for (const auto& kv : palettes) {
					TilesetCategory* tilesetCategory = tileset_ptr->getCategory(kv.second);

					if (kv.second != TILESET_RAW && !tilesetCategory->brushlist.empty()) {
						blocked = 0;
					}
				}

				if (blocked) {
					continue;
				}
			}

			pugi::xml_node tileset = node.append_child("tileset");
			tileset.append_attribute("name") = tileset_ptr->name.c_str();

			for (const auto& kv : palettes) {
				TilesetCategory* tilesetCategory = tileset_ptr->getCategory(kv.second);

				if (!tilesetCategory->brushlist.empty()) {
					std::string data = kv.first;
					std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return std::tolower(c); });

					pugi::xml_node palette = tileset.append_child(data.c_str());
					for (const auto& brush_ptr : tilesetCategory->brushlist) {
						if (!brush_ptr->isRaw()) {
							pugi::xml_node brush = palette.append_child("brush");
							brush.append_attribute("name") = brush_ptr->getName().c_str();
						} else {
							ItemType& it = g_items[brush_ptr->asRaw()->getItemID()];
							if (it.id != 0) {
								pugi::xml_node item = palette.append_child("item");
								item.append_attribute("id") = it.id;
							}
						}
					}
				}
			}

			size_t n = std::distance(tileset.begin(), tileset.end());
			if (n <= 0) {
				node.remove_child(tileset);
			}
		}

		doc.save_file(file.GetFullPath().mb_str());
		DialogUtil::PopupDialog("Successfully saved Tilesets", "Saved tilesets to '" + std::string(file.GetFullPath().mb_str()) + "'", wxOK);
		g_materials.modify(false);
	} catch (std::bad_alloc&) {
		DialogUtil::PopupDialog("Error", "There is not enough memory available to complete the operation.", wxOK);
	}

	g_gui.DestroyLoadBar();
}
