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

#include "editor/selection.h"
#include "editor/selection_thread.h"
#include "map/tile.h"
#include "game/creature.h"
#include "game/item.h"
#include "editor/editor.h"
#include "editor/action_queue.h"
#include "ui/gui.h"

Selection::Selection(Editor& editor) :
	busy(false),
	deferred(false),
	editor(editor),
	session(nullptr),
	subsession(nullptr) {
	////
}

Selection::~Selection() {
	delete subsession;
	delete session;
}

Position Selection::minPosition() const {
	Position minPos(0x10000, 0x10000, 0x10);
	for (TileSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile) {
		Position pos((*tile)->getPosition());
		if (minPos.x > pos.x) {
			minPos.x = pos.x;
		}
		if (minPos.y > pos.y) {
			minPos.y = pos.y;
		}
		if (minPos.z > pos.z) {
			minPos.z = pos.z;
		}
	}
	return minPos;
}

Position Selection::maxPosition() const {
	Position maxPos(0, 0, 0);
	for (TileSet::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile) {
		Position pos((*tile)->getPosition());
		if (maxPos.x < pos.x) {
			maxPos.x = pos.x;
		}
		if (maxPos.y < pos.y) {
			maxPos.y = pos.y;
		}
		if (maxPos.z < pos.z) {
			maxPos.z = pos.z;
		}
	}
	return maxPos;
}

void Selection::add(Tile* tile, Item* item) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	if (item->isSelected()) {
		return;
	}

	// Make a copy of the tile with the item selected
	item->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	item->deselect();

	if (g_settings.getInteger(Config::BORDER_IS_GROUND)) {
		if (item->isBorder()) {
			new_tile->selectGround();
		}
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile, Spawn* spawn) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(spawn);

	if (spawn->isSelected()) {
		return;
	}

	// Make a copy of the tile with the item selected
	spawn->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	spawn->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile, Creature* creature) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(creature);

	if (creature->isSelected()) {
		return;
	}

	// Make a copy of the tile with the item selected
	creature->select();
	Tile* new_tile = tile->deepCopy(editor.map);
	creature->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::add(Tile* tile) {
	ASSERT(subsession);
	ASSERT(tile);

	Tile* new_tile = tile->deepCopy(editor.map);
	new_tile->select();

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Item* item) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(item);

	bool tmp = item->isSelected();
	item->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if (tmp) {
		item->select();
	}
	if (item->isBorder() && g_settings.getInteger(Config::BORDER_IS_GROUND)) {
		new_tile->deselectGround();
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Spawn* spawn) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(spawn);

	bool tmp = spawn->isSelected();
	spawn->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if (tmp) {
		spawn->select();
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile, Creature* creature) {
	ASSERT(subsession);
	ASSERT(tile);
	ASSERT(creature);

	bool tmp = creature->isSelected();
	creature->deselect();
	Tile* new_tile = tile->deepCopy(editor.map);
	if (tmp) {
		creature->select();
	}

	subsession->addChange(newd Change(new_tile));
}

void Selection::remove(Tile* tile) {
	ASSERT(subsession);

	Tile* new_tile = tile->deepCopy(editor.map);
	new_tile->deselect();

	subsession->addChange(newd Change(new_tile));
}

void Selection::addInternal(Tile* tile) {
	ASSERT(tile);

	if (deferred) {
		pending_adds.push_back(tile);
	} else {
		auto it = std::lower_bound(tiles.begin(), tiles.end(), tile, std::less<Tile*>());
		if (it == tiles.end() || *it != tile) {
			tiles.insert(it, tile);
		}
	}
}

void Selection::removeInternal(Tile* tile) {
	ASSERT(tile);
	if (deferred) {
		pending_removes.push_back(tile);
	} else {
		auto it = std::lower_bound(tiles.begin(), tiles.end(), tile, std::less<Tile*>());
		if (it != tiles.end() && *it == tile) {
			tiles.erase(it);
		}
	}
}

void Selection::flush() {
	if (pending_adds.empty() && pending_removes.empty()) {
		return;
	}

	std::sort(pending_adds.begin(), pending_adds.end(), std::less<Tile*>());
	pending_adds.erase(std::unique(pending_adds.begin(), pending_adds.end()), pending_adds.end());

	std::sort(pending_removes.begin(), pending_removes.end(), std::less<Tile*>());
	pending_removes.erase(std::unique(pending_removes.begin(), pending_removes.end()), pending_removes.end());

	TileSet temp;
	temp.reserve(tiles.size());

	// Remove
	std::set_difference(tiles.begin(), tiles.end(), pending_removes.begin(), pending_removes.end(), std::back_inserter(temp), std::less<Tile*>());

	// Add
	tiles.clear();
	tiles.reserve(temp.size() + pending_adds.size());
	std::set_union(temp.begin(), temp.end(), pending_adds.begin(), pending_adds.end(), std::back_inserter(tiles), std::less<Tile*>());

	pending_adds.clear();
	pending_removes.clear();
}

void Selection::clear() {
	if (session) {
		for (TileSet::iterator it = tiles.begin(); it != tiles.end(); it++) {
			Tile* new_tile = (*it)->deepCopy(editor.map);
			new_tile->deselect();
			subsession->addChange(newd Change(new_tile));
		}
	} else {
		for (TileSet::iterator it = tiles.begin(); it != tiles.end(); it++) {
			(*it)->deselect();
		}
		tiles.clear();
	}
}

void Selection::start(SessionFlags flags) {
	if (!(flags & INTERNAL)) {
		if (flags & SUBTHREAD) {
			;
		} else {
			session = editor.actionQueue->createBatch(ACTION_SELECT);
		}
		subsession = editor.actionQueue->createAction(ACTION_SELECT);
	} else {
		deferred = true;
		pending_adds.clear();
		pending_removes.clear();
	}
	busy = true;
}

void Selection::commit() {
	if (session) {
		ASSERT(subsession);
		// We need to step out of the session before we do the action, else peril awaits us!
		BatchAction* tmp = session;
		session = nullptr;

		// Do the action
		tmp->addAndCommitAction(subsession);

		// Create a newd action for subsequent selects
		subsession = editor.actionQueue->createAction(ACTION_SELECT);
		session = tmp;
	}
}

void Selection::finish(SessionFlags flags) {
	if (!(flags & INTERNAL)) {
		if (flags & SUBTHREAD) {
			ASSERT(subsession);
			subsession = nullptr;
		} else {
			ASSERT(session);
			ASSERT(subsession);
			// We need to exit the session before we do the action, else peril awaits us!
			BatchAction* tmp = session;
			session = nullptr;

			tmp->addAndCommitAction(subsession);
			editor.addBatch(tmp, 2);

			session = nullptr;
			subsession = nullptr;
		}
	} else {
		flush();
		deferred = false;
	}
	busy = false;
}

void Selection::updateSelectionCount() {
	if (onSelectionChange) {
		onSelectionChange(size());
	}
}

void Selection::join(SelectionThread* thread) {
	thread->Wait();

	ASSERT(session);
	session->addAction(thread->result);
	thread->selection.subsession = nullptr;

	delete thread;
}
