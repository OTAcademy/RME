//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "game/town.h"

#include "map/map.h"
#include "map/tile.h"

Towns::Towns() {
	////
}

Towns::~Towns() {
	clear();
}

void Towns::clear() {
	towns.clear();
}

bool Towns::addTown(std::unique_ptr<Town> town) {
	TownMap::iterator it = find(town->getID());
	if (it != end()) {
		return false;
	}
	towns[town->getID()] = std::move(town);
	return true;
}

uint32_t Towns::getEmptyID() {
	uint32_t empty = 0;
	for (TownMap::iterator it = begin(); it != end(); ++it) {
		if (it->second->getID() > empty) {
			empty = it->second->getID();
		}
	}
	return empty + 1;
}

Town* Towns::getTown(std::string& name) {
	for (TownMap::iterator it = begin(); it != end(); ++it) {
		if (it->second->getName() == name) {
			return it->second.get();
		}
	}
	return nullptr;
}

Town* Towns::getTown(uint32_t id) {
	TownMap::iterator it = find(id);
	if (it != end()) {
		return it->second.get();
	}
	return nullptr;
}

void Town::setTemplePosition(const Position& pos) {
	templepos = pos;
}
