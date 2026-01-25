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

#ifndef RME_EDITOR_DIRTY_LIST_H
#define RME_EDITOR_DIRTY_LIST_H

#include <vector>
#include <set>
#include <cstdint>

class Change;

// A dirty list represents a list of all tiles that was changed in an action
class DirtyList {
public:
	DirtyList();
	~DirtyList();

	struct ValueType {
		uint32_t pos;
		uint32_t floors;
	};

	uint32_t owner;

protected:
	struct Comparator {
		bool operator()(const ValueType& a, const ValueType& b) const {
			return a.pos < b.pos;
		}
	};

public:
	using SetType = std::set<ValueType, Comparator>;
	using ChangeList = std::vector<Change*>;

	void AddPosition(int x, int y, int z);
	void AddChange(Change* c);
	bool Empty() const {
		return iset.empty() && ichanges.empty();
	}
	SetType& GetPosList();
	ChangeList& GetChanges();

protected:
	SetType iset;
	ChangeList ichanges;
};

#endif
