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

#include "editor/copybuffer.h"
#include "editor/editor.h"
#include "editor/operations/copy_operations.h"
#include "ui/gui.h"
#include "game/creature.h"

CopyBuffer::CopyBuffer() :
	tiles(std::make_unique<BaseMap>()) {
	;
}

size_t CopyBuffer::GetTileCount() {
	return tiles ? (size_t)tiles->size() : 0;
}

BaseMap& CopyBuffer::getBufferMap() {
	ASSERT(tiles);
	return *tiles;
}

CopyBuffer::~CopyBuffer() {
	clear();
}

Position CopyBuffer::getPosition() const {
	ASSERT(tiles);
	return copyPos;
}

void CopyBuffer::clear() {
	tiles.reset();
}

void CopyBuffer::copy(Editor& editor, int floor) {
	CopyOperations::copy(editor, *this, floor);
}

void CopyBuffer::cut(Editor& editor, int floor) {
	CopyOperations::cut(editor, *this, floor);
}

void CopyBuffer::paste(Editor& editor, const Position& toPosition) {
	CopyOperations::paste(editor, *this, toPosition);
}

bool CopyBuffer::canPaste() const {
	return tiles && tiles->size() != 0;
}
