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

#ifndef RME_EDITOR_SELECTION_THREAD_H
#define RME_EDITOR_SELECTION_THREAD_H

#include <wx/thread.h>
#include "map/position.h"
#include "editor/selection.h"

class Editor;
class Action;

class SelectionThread : public wxThread {
public:
	SelectionThread(Editor& editor, Position start, Position end);
	virtual ~SelectionThread();

	void Execute(); // Calls "Create" and then "Run"
protected:
	virtual ExitCode Entry();
	Editor& editor;
	Position start, end;
	Selection selection;
	std::unique_ptr<Action> result;

	friend class Selection;
};

#endif
