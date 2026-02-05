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

#include "editor/action.h"
#include "editor/dirty_list.h"
#include "app/settings.h"
#include "map/map.h"
#include "editor/editor.h"
#include "ui/gui.h"

#include <ranges>
#include <ctime>

Change::Change() :
	type(CHANGE_NONE), data(std::monostate {}) {
	////
}

Change::Change(Tile* t) :
	type(CHANGE_TILE) {
	ASSERT(t);
	data = std::unique_ptr<Tile>(t);
}

Change* Change::Create(House* house, const Position& where) {
	Change* c = newd Change();
	c->type = CHANGE_MOVE_HOUSE_EXIT;
	c->data = HouseExitChangeData { .houseId = house->getID(), .pos = where };
	return c;
}

Change* Change::Create(Waypoint* wp, const Position& where) {
	Change* c = newd Change();
	c->type = CHANGE_MOVE_WAYPOINT;
	c->data = WaypointChangeData { .name = wp->name, .pos = where };
	return c;
}

Change::~Change() {
	clear();
}

void Change::clear() {
	type = CHANGE_NONE;
	data = std::monostate {};
}

const Tile* Change::getTile() const {
	if (auto* t = std::get_if<std::unique_ptr<Tile>>(&data)) {
		return t->get();
	}
	return nullptr;
}

const HouseExitChangeData* Change::getHouseExitData() const {
	return std::get_if<HouseExitChangeData>(&data);
}

const WaypointChangeData* Change::getWaypointData() const {
	return std::get_if<WaypointChangeData>(&data);
}

uint32_t Change::memsize() const {
	uint32_t mem = sizeof(*this);
	if (auto* t = std::get_if<std::unique_ptr<Tile>>(&data)) {
		mem += (*t)->memsize();
	} else if (auto* wp = std::get_if<WaypointChangeData>(&data)) {
		mem += wp->name.capacity();
	} else if (auto* house = std::get_if<HouseExitChangeData>(&data)) {
		mem += sizeof(HouseExitChangeData);
	}
	return mem;
}

Action::Action(Editor& editor, ActionIdentifier ident) :
	commited(false),
	editor(editor),
	type(ident) {
}

Action::~Action() {
	changes.clear();
}

size_t Action::approx_memsize() const {
	uint32_t mem = sizeof(*this);
	mem += changes.size() * (sizeof(Change) + sizeof(Tile) + sizeof(Item) + 6 /* approx overhead*/);
	return mem;
}

size_t Action::memsize() const {
	uint32_t mem = sizeof(*this);
	mem += sizeof(Change*) * 3 * changes.size();
	for (const auto& c : changes) {
		mem += c->memsize();
	}
	return mem;
}

void Action::commit(DirtyList* dirty_list) {
	editor.selection.start(Selection::INTERNAL);
	ChangeList::const_iterator it = changes.begin();
	while (it != changes.end()) {
		Change* c = it->get();
		switch (c->type) {
			case CHANGE_TILE: {
				auto& uptr = std::get<std::unique_ptr<Tile>>(c->data);
				Tile* newtile = uptr.release();
				ASSERT(newtile);
				Position pos = newtile->getPosition();

				if (editor.live_manager.IsClient()) {
					MapNode* nd = editor.map.getLeaf(pos.x, pos.y);
					if (!nd || !nd->isVisible(pos.z > GROUND_LAYER)) {
						// Delete all changes that affect tiles outside our view
						c->clear();
						delete newtile;
						++it;
						continue;
					}
				}

				Tile* oldtile = editor.map.swapTile(pos, newtile);
				TileLocation* location = newtile->getLocation();

				// Update other nodes in the network
				if (editor.live_manager.IsServer() && dirty_list) {
					dirty_list->AddPosition(pos.x, pos.y, pos.z);
				}

				newtile->update();

				if (newtile->isSelected()) {
					editor.selection.addInternal(newtile);
				}

				if (oldtile) {
					if (newtile->getHouseID() != oldtile->getHouseID()) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = editor.map.houses.getHouse(oldtile->getHouseID());
						if (house) {
							house->removeTile(oldtile);
						}

						house = editor.map.houses.getHouse(newtile->getHouseID());
						if (house) {
							house->addTile(newtile);
						}
					}
					if (oldtile->spawn) {
						if (newtile->spawn) {
							if (*oldtile->spawn != *newtile->spawn) {
								editor.map.removeSpawn(oldtile);
								editor.map.addSpawn(newtile);
							}
						} else {
							// Spawn has been removed
							editor.map.removeSpawn(oldtile);
						}
					} else if (newtile->spawn) {
						editor.map.addSpawn(newtile);
					}

					// oldtile->update();
					if (oldtile->isSelected()) {
						editor.selection.removeInternal(oldtile);
					}

					uptr.reset(oldtile);
				} else {
					uptr.reset(editor.map.allocator(location));
					if (newtile->getHouseID() != 0) {
						// oooooomggzzz we need to add it to the appropriate house!
						House* house = editor.map.houses.getHouse(newtile->getHouseID());
						if (house) {
							house->addTile(newtile);
						}
					}

					if (newtile->spawn) {
						editor.map.addSpawn(newtile);
					}
				}
				// Mark the tile as modified
				newtile->modify();

				// Update client dirty list
				if (editor.live_manager.IsClient() && dirty_list && type != ACTION_REMOTE) {
					// Local action, assemble changes
					dirty_list->AddChange(c);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				auto& p = std::get<HouseExitChangeData>(c->data);
				House* whathouse = editor.map.houses.getHouse(p.houseId);

				if (whathouse) {
					Position oldpos = whathouse->getExit();
					whathouse->setExit(p.pos);
					p.pos = oldpos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				auto& p = std::get<WaypointChangeData>(c->data);
				Waypoint* wp = editor.map.waypoints.getWaypoint(p.name);

				if (wp) {
					// Change the tiles
					TileLocation* oldtile = editor.map.getTileL(wp->pos);
					TileLocation* newtile = editor.map.getTileL(p.pos);

					// Only need to remove from old if it actually exists
					if (p.pos != Position()) {
						if (oldtile && oldtile->getWaypointCount() > 0) {
							oldtile->decreaseWaypointCount();
						}
					}

					newtile->increaseWaypointCount();

					// Update shit
					Position oldpos = wp->pos;
					wp->pos = p.pos;
					p.pos = oldpos;
				}
				break;
			}

			default:
				break;
		}
		++it;
	}
	editor.selection.finish(Selection::INTERNAL);
	commited = true;
}

void Action::undo(DirtyList* dirty_list) {
	if (changes.empty()) {
		return;
	}

	editor.selection.start(Selection::INTERNAL);
	ChangeList::reverse_iterator it = changes.rbegin();

	while (it != changes.rend()) {
		Change* c = it->get();
		switch (c->type) {
			case CHANGE_TILE: {
				auto& uptr = std::get<std::unique_ptr<Tile>>(c->data);
				Tile* oldtile = uptr.release();
				ASSERT(oldtile);
				Position pos = oldtile->getPosition();

				if (editor.live_manager.IsClient()) {
					MapNode* nd = editor.map.getLeaf(pos.x, pos.y);
					if (!nd || !nd->isVisible(pos.z > GROUND_LAYER)) {
						// Delete all changes that affect tiles outside our view
						c->clear();
						delete oldtile;
						++it;
						continue;
					}
				}

				Tile* newtile = editor.map.swapTile(pos, oldtile);

				// Update server side change list (for broadcast)
				if (editor.live_manager.IsServer() && dirty_list) {
					dirty_list->AddPosition(pos.x, pos.y, pos.z);
				}

				if (oldtile->isSelected()) {
					editor.selection.addInternal(oldtile);
				}
				if (newtile->isSelected()) {
					editor.selection.removeInternal(newtile);
				}

				if (newtile->getHouseID() != oldtile->getHouseID()) {
					// oooooomggzzz we need to remove it from the appropriate house!
					House* house = editor.map.houses.getHouse(newtile->getHouseID());
					if (house) {
						house->removeTile(newtile);
					} else {
						// Set tile house to 0, house has been removed
						newtile->setHouse(nullptr);
					}

					house = editor.map.houses.getHouse(oldtile->getHouseID());
					if (house) {
						house->addTile(oldtile);
					}
				}

				if (oldtile->spawn) {
					if (newtile->spawn) {
						if (*oldtile->spawn != *newtile->spawn) {
							editor.map.removeSpawn(newtile);
							editor.map.addSpawn(oldtile);
						}
					} else {
						editor.map.addSpawn(oldtile);
					}
				} else if (newtile->spawn) {
					editor.map.removeSpawn(newtile);
				}
				uptr.reset(newtile);

				// Update client dirty list
				if (editor.live_manager.IsClient() && dirty_list && type != ACTION_REMOTE) {
					// Local action, assemble changes
					dirty_list->AddChange(c);
				}
				break;
			}

			case CHANGE_MOVE_HOUSE_EXIT: {
				auto& p = std::get<HouseExitChangeData>(c->data);
				House* whathouse = editor.map.houses.getHouse(p.houseId);
				if (whathouse) {
					Position oldpos = whathouse->getExit();
					whathouse->setExit(p.pos);
					p.pos = oldpos;
				}
				break;
			}

			case CHANGE_MOVE_WAYPOINT: {
				auto& p = std::get<WaypointChangeData>(c->data);
				Waypoint* wp = editor.map.waypoints.getWaypoint(p.name);

				if (wp) {
					// Change the tiles
					TileLocation* oldtile = editor.map.getTileL(wp->pos);
					TileLocation* newtile = editor.map.getTileL(p.pos);

					// Only need to remove from old if it actually exists
					if (p.pos != Position()) {
						if (oldtile && oldtile->getWaypointCount() > 0) {
							oldtile->decreaseWaypointCount();
						}
					}

					if (newtile) {
						newtile->increaseWaypointCount();
					}

					// Update shit
					Position oldpos = wp->pos;
					wp->pos = p.pos;
					p.pos = oldpos;
				}
				break;
			}

			default:
				break;
		}
		++it;
	}
	editor.selection.finish(Selection::INTERNAL);
	commited = false;
}

BatchAction::BatchAction(Editor& editor, ActionIdentifier ident) :
	editor(editor),
	timestamp(0),
	memory_size(0),
	type(ident) {
	////
}

BatchAction::~BatchAction() {
	// batch is vector<unique_ptr>, destruction handled automatically
	batch.clear();
}

size_t BatchAction::memsize(bool recalc) const {
	// Expensive operation, only evaluate once (won't change anyways)
	if (!recalc && memory_size > 0) {
		return memory_size;
	}

	uint32_t mem = sizeof(*this);
	mem += sizeof(Action*) * 3 * batch.size();

	for (const auto& action : batch) {
#ifdef __USE_EXACT_MEMSIZE__
		mem += action->memsize();
#else
		// Less exact but MUCH faster
		mem += action->approx_memsize();
#endif
	}

	const_cast<BatchAction*>(this)->memory_size = mem;
	return mem;
}

void BatchAction::addAction(std::unique_ptr<Action> action) {
	// If empty, do nothing.
	if (action->size() == 0) {
		return;
	}

	ASSERT(action->getType() == type);

	if (editor.live_manager.IsClient()) {
		return;
	}

	// Add it!
	batch.push_back(std::move(action));
	timestamp = time(nullptr);
}

void BatchAction::addAndCommitAction(std::unique_ptr<Action> action) {
	// If empty, do nothing.
	if (action->size() == 0) {
		return;
	}

	if (editor.live_manager.IsClient()) {
		return;
	}

	// Add it!
	action->commit(nullptr);
	batch.push_back(std::move(action));
	timestamp = time(nullptr);
}

void BatchAction::commit() {
	for (const auto& action : batch) {
		if (!action->isCommited()) {
			action->commit(nullptr);
		}
	}
}

void BatchAction::undo() {
	for (auto& action : std::ranges::reverse_view(batch)) {
		action->undo(nullptr);
	}
}

void BatchAction::redo() {
	for (auto& action : batch) {
		action->redo(nullptr);
	}
}

void BatchAction::merge(BatchAction* other) {
	batch.insert(batch.end(), std::make_move_iterator(other->batch.begin()), std::make_move_iterator(other->batch.end()));
	other->batch.clear();
}
