//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "ui/dialog_util.h"
#include "util/file_system.h"
#include "editor/persistence/editor_persistence.h"
#include "editor/editor.h"
#include "editor/action.h"
#include "editor/action_queue.h"
#include "editor/selection.h"
#include "map/map.h"
#include "io/iomap.h"
#include "app/settings.h"
#include "app/managers/version_manager.h"
#include "ui/gui.h"

#include <fstream>
#include <ctime>
#include <sstream>
#include <format>
#include <spdlog/spdlog.h>

void EditorPersistence::loadMap(Editor& editor, const FileName& fn) {
	MapVersion ver;
	if (!IOMapOTBM::getVersionInfo(fn, ver)) {
		throw std::runtime_error("Could not open file \"" + nstr(fn.GetFullPath()) + "\".\nThis is not a valid OTBM file or it does not exist.");
	}

	bool success = true;
	if (g_version.GetCurrentVersionID() != ver.client) {
		throw std::runtime_error(std::format("Client version mismatch. Expected {} but got {}", ver.client, g_version.GetCurrentVersionID()));
	}

	if (success) {
		ScopedLoadingBar LoadingBar("Loading OTBM map...");
		success = editor.map.open(nstr(fn.GetFullPath()));
	}
}

void EditorPersistence::saveMap(Editor& editor, FileName filename, bool showdialog) {
	std::string savefile = filename.GetFullPath().mb_str(wxConvUTF8).data();
	bool save_as = false;
	bool save_otgz = false;

	if (savefile.empty()) {
		savefile = editor.map.getFilename();

		FileName c1(wxstr(savefile));
		FileName c2(wxstr(editor.map.getFilename()));
		save_as = c1 != c2;
	}

	// If not named yet, propagate the file name to the auxilliary files
	if (editor.map.unnamed) {
		FileName _name(filename);
		_name.SetExt("xml");

		_name.SetName(filename.GetName() + "-spawn");
		editor.map.spawnfile = nstr(_name.GetFullName());
		_name.SetName(filename.GetName() + "-house");
		editor.map.housefile = nstr(_name.GetFullName());
		_name.SetName(filename.GetName() + "-waypoint");
		editor.map.waypointfile = nstr(_name.GetFullName());

		editor.map.unnamed = false;
	}

	// File object to convert between local paths etc.
	FileName converter;
	converter.Assign(wxstr(savefile));
	std::string map_path = nstr(converter.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));

	// Make temporary backups
	// converter.Assign(wxstr(savefile));
	std::string backup_otbm, backup_house, backup_spawn, backup_waypoint;

	if (converter.GetExt() == "otgz") {
		save_otgz = true;
		if (converter.FileExists()) {
			backup_otbm = map_path + nstr(converter.GetName()) + ".otgz~";
			std::remove(backup_otbm.c_str());
			std::rename(savefile.c_str(), backup_otbm.c_str());
		}
	} else {
		if (converter.FileExists()) {
			backup_otbm = map_path + nstr(converter.GetName()) + ".otbm~";
			std::remove(backup_otbm.c_str());
			std::rename(savefile.c_str(), backup_otbm.c_str());
		}

		converter.SetFullName(wxstr(editor.map.getHouseFilename()));
		if (converter.FileExists()) {
			backup_house = map_path + nstr(converter.GetName()) + ".xml~";
			std::remove(backup_house.c_str());
			std::rename((map_path + editor.map.getHouseFilename()).c_str(), backup_house.c_str());
		}

		converter.SetFullName(wxstr(editor.map.getSpawnFilename()));
		if (converter.FileExists()) {
			backup_spawn = map_path + nstr(converter.GetName()) + ".xml~";
			std::remove(backup_spawn.c_str());
			std::rename((map_path + editor.map.getSpawnFilename()).c_str(), backup_spawn.c_str());
		}

		converter.SetFullName(wxstr(editor.map.waypointfile));
		if (converter.FileExists()) {
			backup_waypoint = map_path + nstr(converter.GetName()) + ".xml~";
			std::remove(backup_waypoint.c_str());
			std::rename((map_path + editor.map.waypointfile).c_str(), backup_waypoint.c_str());
		}
	}

	// Save the map
	{
		std::string n = nstr(FileSystem::GetLocalDataDirectory()) + ".saving.txt";
		std::ofstream f(n.c_str(), std::ios::trunc | std::ios::out);
		f << backup_otbm << std::endl
		  << backup_house << std::endl
		  << backup_spawn << std::endl;
	}

	{

		// Set up the Map paths
		wxFileName fn = wxstr(savefile);
		editor.map.filename = fn.GetFullPath().mb_str(wxConvUTF8);
		editor.map.name = fn.GetFullName().mb_str(wxConvUTF8);

		if (showdialog) {
			g_gui.CreateLoadBar("Saving OTBM map...");
		}

		// Perform the actual save
		IOMapOTBM mapsaver(editor.map.getVersion());
		bool success = mapsaver.saveMap(editor.map, fn);

		if (showdialog) {
			g_gui.DestroyLoadBar();
		}

		// Check for errors...
		if (!success) {
			// Rename the temporary backup files back to their previous names
			if (!backup_otbm.empty()) {
				converter.SetFullName(wxstr(savefile));
				std::string otbm_filename = map_path + nstr(converter.GetName());
				std::rename(backup_otbm.c_str(), std::string(otbm_filename + (save_otgz ? ".otgz" : ".otbm")).c_str());
			}

			if (!backup_house.empty()) {
				converter.SetFullName(wxstr(editor.map.getHouseFilename()));
				std::string house_filename = map_path + nstr(converter.GetName());
				std::rename(backup_house.c_str(), std::string(house_filename + ".xml").c_str());
			}

			if (!backup_spawn.empty()) {
				converter.SetFullName(wxstr(editor.map.getSpawnFilename()));
				std::string spawn_filename = map_path + nstr(converter.GetName());
				std::rename(backup_spawn.c_str(), std::string(spawn_filename + ".xml").c_str());
			}

			if (!backup_waypoint.empty()) {
				converter.SetFullName(wxstr(editor.map.waypointfile));
				std::string waypoint_filename = map_path + nstr(converter.GetName());
				std::rename(backup_waypoint.c_str(), std::string(waypoint_filename + ".xml").c_str());
			}

			// Display the error
			DialogUtil::PopupDialog("Error", "Could not save, unable to open target for writing.", wxOK);
		}

		// Remove temporary save runfile
		{
			std::string n = nstr(FileSystem::GetLocalDataDirectory()) + ".saving.txt";
			std::remove(n.c_str());
		}

		// If failure, don't run the rest of the function
		if (!success) {
			return;
		}
	}

	// Move to permanent backup
	if (!save_as && g_settings.getInteger(Config::ALWAYS_MAKE_BACKUP)) {
		// Move temporary backups to their proper files
		time_t t = time(nullptr);
		tm* current_time = localtime(&t);
		ASSERT(current_time);

		std::ostringstream date;
		date << (1900 + current_time->tm_year);
		if (current_time->tm_mon < 9) {
			date << "-"
				 << "0" << current_time->tm_mon + 1;
		} else {
			date << "-" << current_time->tm_mon + 1;
		}
		date << "-" << current_time->tm_mday;
		date << "-" << current_time->tm_hour;
		date << "-" << current_time->tm_min;
		date << "-" << current_time->tm_sec;

		if (!backup_otbm.empty()) {
			converter.SetFullName(wxstr(savefile));
			std::string otbm_filename = map_path + nstr(converter.GetName());
			std::rename(backup_otbm.c_str(), std::string(otbm_filename + "." + date.str() + (save_otgz ? ".otgz" : ".otbm")).c_str());
		}

		if (!backup_house.empty()) {
			converter.SetFullName(wxstr(editor.map.getHouseFilename()));
			std::string house_filename = map_path + nstr(converter.GetName());
			std::rename(backup_house.c_str(), std::string(house_filename + "." + date.str() + ".xml").c_str());
		}

		if (!backup_spawn.empty()) {
			converter.SetFullName(wxstr(editor.map.getSpawnFilename()));
			std::string spawn_filename = map_path + nstr(converter.GetName());
			std::rename(backup_spawn.c_str(), std::string(spawn_filename + "." + date.str() + ".xml").c_str());
		}

		if (!backup_waypoint.empty()) {
			converter.SetFullName(wxstr(editor.map.getSpawnFilename()));
			std::string waypoint_filename = map_path + nstr(converter.GetName());
			std::rename(backup_waypoint.c_str(), std::string(waypoint_filename + "." + date.str() + ".xml").c_str());
		}
	} else {
		// Delete the temporary files
		std::remove(backup_otbm.c_str());
		std::remove(backup_house.c_str());
		std::remove(backup_spawn.c_str());
	}

	editor.map.clearChanges();
}

bool EditorPersistence::importMap(Editor& editor, FileName filename, int import_x_offset, int import_y_offset, ImportType house_import_type, ImportType spawn_import_type) {
	editor.selection.clear();
	editor.actionQueue->clear();

	Map imported_map;
	bool loaded = imported_map.open(nstr(filename.GetFullPath()));

	if (!loaded) {
		DialogUtil::PopupDialog("Error", "Error loading map!\n" + imported_map.getError(), wxOK | wxICON_INFORMATION);
		return false;
	}
	DialogUtil::ListDialog("Warning", imported_map.getWarnings());

	Position offset(import_x_offset, import_y_offset, 0);

	bool resizemap = false;
	bool resize_asked = false;
	int newsize_x = editor.map.getWidth(), newsize_y = editor.map.getHeight();
	int discarded_tiles = 0;

	g_gui.CreateLoadBar("Merging maps...");

	std::map<uint32_t, uint32_t> town_id_map;
	std::map<uint32_t, uint32_t> house_id_map;

	if (house_import_type != IMPORT_DONT) {
		for (TownMap::iterator tit = imported_map.towns.begin(); tit != imported_map.towns.end();) {
			Town* imported_town = tit->second.get();
			Town* current_town = editor.map.towns.getTown(imported_town->getID());

			Position oldexit = imported_town->getTemplePosition();
			Position newexit = oldexit + offset;
			if (newexit.isValid()) {
				imported_town->setTemplePosition(newexit);
				editor.map.getOrCreateTile(newexit)->getLocation()->increaseTownCount();
			}

			switch (house_import_type) {
				case IMPORT_MERGE: {
					town_id_map[imported_town->getID()] = imported_town->getID();
					if (current_town) {
						++tit;
						continue;
					}
					break;
				}
				case IMPORT_SMART_MERGE: {
					if (current_town) {
						// Compare and insert/merge depending on parameters
						if (current_town->getName() == imported_town->getName() && current_town->getID() == imported_town->getID()) {
							// Just add to map
							town_id_map[imported_town->getID()] = current_town->getID();
							++tit;
							continue;
						} else {
							// Conflict! Find a newd id and replace old
							uint32_t new_id = editor.map.towns.getEmptyID();
							imported_town->setID(new_id);
							town_id_map[imported_town->getID()] = new_id;
						}
					} else {
						town_id_map[imported_town->getID()] = imported_town->getID();
					}
					break;
				}
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = editor.map.towns.getEmptyID();
					imported_town->setID(new_id);
					town_id_map[imported_town->getID()] = new_id;
					break;
				}
				case IMPORT_DONT: {
					++tit;
					continue; // Should never happend..?
					break; // Continue or break ?
				}
			}

			if (!editor.map.towns.addTown(std::move(tit->second))) {
				spdlog::warn("Failed to add town {} during import (duplicate ID)", imported_town->getID());
			}

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			tit = imported_map.towns.erase(tit);
#else // Bulky, slow way
			TownMap::iterator tmp_iter = tit;
			++tmp_iter;
			uint32_t next_key = 0;
			if (tmp_iter != imported_map.towns.end()) {
				next_key = tmp_iter->first;
			}
			imported_map.towns.erase(tit);
			if (next_key != 0) {
				tit = imported_map.towns.find(next_key);
			} else {
				tit = imported_map.towns.end();
			}
#endif
		}

		for (HouseMap::iterator hit = imported_map.houses.begin(); hit != imported_map.houses.end();) {
			House* imported_house = hit->second.get();
			House* current_house = editor.map.houses.getHouse(imported_house->getID());
			imported_house->townid = town_id_map[imported_house->townid];

			Position oldexit = imported_house->getExit();
			imported_house->setExit(nullptr, Position()); // Reset it

			switch (house_import_type) {
				case IMPORT_MERGE: {
					house_id_map[imported_house->getID()] = imported_house->getID();
					if (current_house) {
						++hit;
						Position newexit = oldexit + offset;
						if (newexit.isValid()) {
							current_house->setExit(&editor.map, newexit);
						}
						continue;
					}
					break;
				}
				case IMPORT_SMART_MERGE: {
					if (current_house) {
						// Compare and insert/merge depending on parameters
						if (current_house->name == imported_house->name && current_house->townid == imported_house->townid) {
							// Just add to map
							house_id_map[imported_house->getID()] = current_house->getID();
							++hit;
							Position newexit = oldexit + offset;
							if (newexit.isValid()) {
								imported_house->setExit(&editor.map, newexit);
							}
							continue;
						} else {
							// Conflict! Find a newd id and replace old
							uint32_t new_id = editor.map.houses.getEmptyID();
							house_id_map[imported_house->getID()] = new_id;
							imported_house->setID(new_id);
						}
					} else {
						house_id_map[imported_house->getID()] = imported_house->getID();
					}
					break;
				}
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = editor.map.houses.getEmptyID();
					house_id_map[imported_house->getID()] = new_id;
					imported_house->setID(new_id);
					break;
				}
				case IMPORT_DONT: {
					++hit;
					Position newexit = oldexit + offset;
					if (newexit.isValid()) {
						imported_house->setExit(&editor.map, newexit);
					}
					continue; // Should never happend..?
					break; // Continue or break ?
				}
			}

			Position newexit = oldexit + offset;
			if (newexit.isValid()) {
				imported_house->setExit(&editor.map, newexit);
			}
			if (!editor.map.houses.addHouse(std::move(hit->second))) {
				spdlog::warn("Failed to add house {} during import (duplicate ID)", imported_house->getID());
			}

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			hit = imported_map.houses.erase(hit);
#else // Bulky, slow way
			HouseMap::iterator tmp_iter = hit;
			++tmp_iter;
			uint32_t next_key = 0;
			if (tmp_iter != imported_map.houses.end()) {
				next_key = tmp_iter->first;
			}
			imported_map.houses.erase(hit);
			if (next_key != 0) {
				hit = imported_map.houses.find(next_key);
			} else {
				hit = imported_map.houses.end();
			}
#endif
		}
	}

	std::map<Position, Spawn*> spawn_map;
	if (spawn_import_type != IMPORT_DONT) {
		for (SpawnPositionList::iterator siter = imported_map.spawns.begin(); siter != imported_map.spawns.end();) {
			Position old_spawn_pos = *siter;
			Position new_spawn_pos = *siter + offset;
			switch (spawn_import_type) {
				case IMPORT_SMART_MERGE:
				case IMPORT_INSERT:
				case IMPORT_MERGE: {
					Tile* imported_tile = imported_map.getTile(old_spawn_pos);
					if (imported_tile) {
						ASSERT(imported_tile->spawn);
						spawn_map[new_spawn_pos] = imported_tile->spawn;

						SpawnPositionList::iterator next = siter;
						bool cont = true;
						Position next_spawn;

						++next;
						if (next == imported_map.spawns.end()) {
							cont = false;
						} else {
							next_spawn = *next;
						}
						imported_map.spawns.erase(siter);
						if (cont) {
							siter = imported_map.spawns.find(next_spawn);
						} else {
							siter = imported_map.spawns.end();
						}
					}
					break;
				}
				case IMPORT_DONT: {
					++siter;
					break;
				}
			}
		}
	}

	// Plain merge of waypoints, very simple! :)
	for (WaypointMap::iterator iter = imported_map.waypoints.begin(); iter != imported_map.waypoints.end(); ++iter) {
		iter->second->pos += offset;
	}

	editor.map.waypoints.waypoints.insert(imported_map.waypoints.begin(), imported_map.waypoints.end());
	imported_map.waypoints.waypoints.clear();

	uint64_t tiles_merged = 0;
	uint64_t tiles_to_import = imported_map.getTileCount();
	for (MapIterator mit = imported_map.begin(); mit != imported_map.end(); ++mit) {
		if (tiles_merged % 8092 == 0) {
			g_gui.SetLoadDone(int(100.0 * tiles_merged / tiles_to_import));
		}
		++tiles_merged;

		Tile* import_tile = mit->get();
		Position new_pos = import_tile->getPosition() + offset;
		if (!new_pos.isValid()) {
			++discarded_tiles;
			continue;
		}

		if (!resizemap && (new_pos.x > editor.map.getWidth() || new_pos.y > editor.map.getHeight())) {
			if (resize_asked) {
				++discarded_tiles;
				continue;
			} else {
				resize_asked = true;
				int ret = DialogUtil::PopupDialog("Collision", "The imported tiles are outside the current map scope. Do you want to resize the map? (Else additional tiles will be removed)", wxYES | wxNO);

				if (ret == wxID_YES) {
					// ...
					resizemap = true;
				} else {
					++discarded_tiles;
					continue;
				}
			}
		}

		if (new_pos.x > newsize_x) {
			newsize_x = new_pos.x;
		}
		if (new_pos.y > newsize_y) {
			newsize_y = new_pos.y;
		}

		imported_map.setTile(import_tile->getPosition(), nullptr);
		TileLocation* location = editor.map.createTileL(new_pos);

		// Check if we should update any houses
		int new_houseid = house_id_map[import_tile->getHouseID()];
		House* house = editor.map.houses.getHouse(new_houseid);
		if (import_tile->isHouseTile() && house_import_type != IMPORT_DONT && house) {
			// We need to notify houses of the tile moving
			house->removeTile(import_tile);
			import_tile->setLocation(location);
			house->addTile(import_tile);
		} else {
			import_tile->setLocation(location);
		}

		if (offset != Position(0, 0, 0)) {
			for (ItemVector::iterator iter = import_tile->items.begin(); iter != import_tile->items.end(); ++iter) {
				Item* item = *iter;
				if (Teleport* teleport = dynamic_cast<Teleport*>(item)) {
					teleport->setDestination(teleport->getDestination() + offset);
				}
			}
		}

		Tile* old_tile = editor.map.getTile(new_pos);
		if (old_tile) {
			editor.map.removeSpawn(old_tile);
		}
		import_tile->spawn = nullptr;

		editor.map.setTile(new_pos, import_tile, true);
	}

	for (std::map<Position, Spawn*>::iterator spawn_iter = spawn_map.begin(); spawn_iter != spawn_map.end(); ++spawn_iter) {
		Position pos = spawn_iter->first;
		TileLocation* location = editor.map.createTileL(pos);
		Tile* tile = location->get();
		if (!tile) {
			tile = editor.map.allocator(location);
			editor.map.setTile(pos, tile);
		} else if (tile->spawn) {
			editor.map.removeSpawnInternal(tile);
			delete tile->spawn;
		}
		tile->spawn = spawn_iter->second;

		editor.map.addSpawn(tile);
	}

	g_gui.DestroyLoadBar();

	editor.map.setWidth(newsize_x);
	editor.map.setHeight(newsize_y);
	DialogUtil::PopupDialog("Success", "Map imported successfully, " + i2ws(discarded_tiles) + " tiles were discarded as invalid.", wxOK);

	g_gui.RefreshPalettes();
	g_gui.FitViewToMap();

	return true;
}

bool EditorPersistence::importMiniMap(Editor& editor, FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset) {
	return false;
}
