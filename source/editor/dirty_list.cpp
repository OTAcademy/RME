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
#include "editor/dirty_list.h"
#include "editor/action.h"

DirtyList::DirtyList() :
	owner(0) {
	;
}

DirtyList::~DirtyList() {
	;
}

void DirtyList::AddPosition(int x, int y, int z) {
	uint32_t m = ((x >> 2) << 18) | ((y >> 2) << 4);
	ValueType fi = { m, 0 };
	SetType::iterator s = iset.find(fi);
	if (s != iset.end()) {
		ValueType v = *s;
		iset.erase(s);
		v.floors = (1 << z) | v.floors;
		iset.insert(v);
	} else {
		ValueType v = { m, (uint32_t)(1 << z) };
		iset.insert(v);
	}
}

void DirtyList::AddChange(Change* c) {
	ichanges.push_back(c);
}

DirtyList::SetType& DirtyList::GetPosList() {
	return iset;
}

DirtyList::ChangeList& DirtyList::GetChanges() {
	return ichanges;
}
