#ifndef RME_ITEM_GRID_PANEL_H_
#define RME_ITEM_GRID_PANEL_H_

#include "util/virtual_item_grid.h"
#include <vector>
#include <map>

class ItemGridPanel : public VirtualItemGrid {
public:
	class Listener {
	public:
		virtual ~Listener() = default;
		virtual void OnItemSelected(ItemGridPanel* source, uint16_t itemId) = 0;
	};

	ItemGridPanel(wxWindow* parent, Listener* listener);
	virtual ~ItemGridPanel();

	// Data Management
	void SetItems(const std::vector<uint16_t>& items);
	void SetFilter(const wxString& filter);

	// VirtualItemGrid Implementation
	size_t GetItemCount() const override;
	uint16_t GetItem(size_t index) const override;
	wxString GetItemName(size_t index) const override;
	void OnItemSelected(int index) override;

	// Overrides
	void OnMotion(wxMouseEvent& event) override; // Drag&Drop Hook

	// Custom properties
	void SetDraggable(bool check);
	void SetOverrideNames(const std::map<uint16_t, wxString>& names);
	void SetShowDetails(bool show);

	uint16_t GetSelectedId() const {
		return (uint16_t)GetSelectedItemId();
	}

protected:
	Listener* listener;

	std::vector<uint16_t> allItems;
	std::vector<uint16_t> filteredItems;

	bool m_draggable = false;
	bool m_showDetails = true;
	std::map<uint16_t, wxString> m_nameOverrides;
};

#endif
