#include "ui/replace_tool/item_grid_panel.h"
#include "ui/theme.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include <wx/dcclient.h>
#include <wx/graphics.h>
#include <wx/dnd.h>
#include <wx/dataobj.h>
#include <algorithm>
#include <format>
#include "rendering/core/text_renderer.h"

ItemGridPanel::ItemGridPanel(wxWindow* parent, Listener* listener) :
	VirtualItemGrid(parent, wxID_ANY),
	listener(listener),
	m_draggable(false),
	m_showDetails(true) {
	// Base class handles bindings
}

ItemGridPanel::~ItemGridPanel() {
}

void ItemGridPanel::SetItems(const std::vector<uint16_t>& items) {
	allItems = items;
	m_nameOverrides.clear();
	ClearCache(); // Clear old textures
	SetFilter("");
}

void ItemGridPanel::SetFilter(const wxString& filter) {
	filteredItems.clear();
	wxString lowerFilter = filter.Lower();
	for (uint16_t id : allItems) {
		if (id < 100) {
			continue;
		}
		const ItemType& it = g_items.getItemType(id);
		if (lowerFilter.IsEmpty()) {
			filteredItems.push_back(id);
		} else {
			wxString name = wxString(it.name);
			if (name.Lower().Contains(lowerFilter) || wxString(std::format("{}", id)).Contains(lowerFilter) || wxString(std::format("{}", it.clientID)).Contains(lowerFilter)) {
				filteredItems.push_back(id);
			}
		}
	}
	SetScrollPosition(0);
	RefreshGrid();
}

size_t ItemGridPanel::GetItemCount() const {
	return filteredItems.size();
}

uint16_t ItemGridPanel::GetItem(size_t index) const {
	if (index < filteredItems.size()) {
		return filteredItems[index];
	}
	return 0;
}

wxString ItemGridPanel::GetItemName(size_t index) const {
	uint16_t id = GetItem(index);
	auto it = m_nameOverrides.find(id);
	if (it != m_nameOverrides.end()) {
		return it->second;
	}
	return wxString(std::format("{} - {}", id, g_items.getItemType(id).name));
}

void ItemGridPanel::OnItemSelected(int index) {
	if (index >= 0 && index < (int)filteredItems.size()) {
		uint16_t id = filteredItems[index];
		if (listener) {
			listener->OnItemSelected(this, id);
		}
	}
}

void ItemGridPanel::OnMotion(wxMouseEvent& event) {
	// Call base to handle hover effects
	VirtualItemGrid::OnMotion(event);

	// Drag Check
	if (event.Dragging() && m_draggable) {
		uint16_t id = GetSelectedItemId();
		if (id != 0) {
			wxTextDataObject data(wxString(std::format("RME_ITEM:{}", id)));
			wxDropSource dragSource(this);
			dragSource.SetData(data);
			dragSource.DoDragDrop(wxDrag_AllowMove);
		}
	}
}

void ItemGridPanel::SetOverrideNames(const std::map<uint16_t, wxString>& names) {
	m_nameOverrides = names;
	Refresh();
}

void ItemGridPanel::SetShowDetails(bool show) {
	m_showDetails = show;
	Refresh();
}

void ItemGridPanel::SetDraggable(bool check) {
	m_draggable = check;
}
