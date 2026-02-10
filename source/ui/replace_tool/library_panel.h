#ifndef RME_LIBRARY_PANEL_H_
#define RME_LIBRARY_PANEL_H_

#include "ui/replace_tool/item_grid_panel.h"
#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/srchctrl.h>
#include <vector>
#include <string>
#include <unordered_map>

class Brush;
class LibraryPanel : public wxPanel, public ItemGridPanel::Listener {
public:
	class Listener {
	public:
		virtual ~Listener() { }
		virtual void OnLibraryItemSelected(uint16_t itemId) = 0;
	};

	LibraryPanel(wxWindow* parent, Listener* listener);
	virtual ~LibraryPanel();

	// ItemGridPanel::Listener
	void OnItemSelected(ItemGridPanel* source, uint16_t itemId) override;

private:
	void InitLayout();
	void PopulateBrushGrid();
	void PopulateRelatedItems(uint16_t brushLookId);
	void OnSearchChange(wxCommandEvent& event);
	void OnBrushSearchChange(wxCommandEvent& event);
	uint16_t GetSidFromCid(uint16_t cid);

	Listener* m_listener;

	wxNotebook* m_notebook;
	wxSearchCtrl* m_itemSearch;
	wxSearchCtrl* m_brushSearch;
	ItemGridPanel* m_allItemsGrid;
	ItemGridPanel* m_brushGrid;
	ItemGridPanel* m_relatedGrid;

	std::unordered_map<uint16_t, uint16_t> m_cidToSidCache;
	std::unordered_map<uint16_t, Brush*> m_brushLookup;
};

#endif
