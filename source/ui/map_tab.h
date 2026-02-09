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

#ifndef RME_MAP_TAB_H_
#define RME_MAP_TAB_H_

#include "editor/editor_tabs.h"
#include "app/application.h"
#include "ui/map_window.h"
#include "editor/map_session.h"

class MapTab : public EditorTab, public MapWindow {
public:
	MapTab(MapTabbook* aui, Editor* editor);
	// Constructs a newd window, but it uses the same internal editor as 'other'
	// AND the same parent, aui_notebook etc.
	MapTab(const MapTab* other);
	~MapTab();

	bool IsUniqueReference() const;
	bool HasSameReference(const MapTab* other) const;

	// Properties
	MapWindow* GetView() const;
	MapCanvas* GetCanvas() const;
	wxWindow* GetWindow() const;
	wxString GetTitle() const;
	Editor* GetEditor() const;
	Map* GetMap() const;
	MapSession* GetSession() const {
		return &iref->session;
	}

	void VisibilityCheck();

	// Event handlers
	void OnSwitchEditorMode(EditorMode mode);
	EditorMode GetMode() const {
		return iref->session.mode;
	}

protected:
	// Session data
	// Moved to MapSession
	struct InternalReference {
		InternalReference(Editor* editor) : session(editor), owner_count(1) { }
		MapSession session;
		int owner_count;
	};
	MapTabbook* aui;
	InternalReference* iref;
};

inline bool MapTab::HasSameReference(const MapTab* other) const {
	return other->iref == iref;
}

#endif
