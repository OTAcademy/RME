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

#ifndef RME_EDITOR_ACTION_QUEUE_H
#define RME_EDITOR_ACTION_QUEUE_H

#include <deque>
#include <vector>
#include "editor/action.h"

class Editor;
class Action;
class BatchAction;

class ActionQueue {
public:
	ActionQueue(Editor& editor);
	virtual ~ActionQueue();

	using ActionList = std::deque<BatchAction*>;

	void resetTimer();

	virtual Action* createAction(ActionIdentifier ident);
	virtual Action* createAction(BatchAction* parent);
	virtual BatchAction* createBatch(ActionIdentifier ident);

	void addBatch(BatchAction* action, int stacking_delay = 0);
	void addAction(Action* action, int stacking_delay = 0);

	void undo();
	void redo();
	void clear();

	bool canUndo() {
		return current > 0;
	}
	bool canRedo() {
		return current < actions.size();
	}

protected:
	size_t current;
	size_t memory_size;
	Editor& editor;
	ActionList actions;
};

#endif
