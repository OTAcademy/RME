#include "app/main.h"
#include <wx/log.h>
#include "ui/replace_tool/library_panel.h"
#include "ui/replace_tool/visual_similarity_service.h"
#include "ui/theme.h"
#include "game/items.h"
#include "game/item.h"
#include "brushes/brush.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/wall/wall_brush.h"
#include "brushes/doodad/doodad_brush.h"
#include "brushes/table/table_brush.h"
#include "brushes/carpet/carpet_brush.h"
#include <wx/splitter.h>
#include <algorithm>

LibraryPanel::LibraryPanel(wxWindow* parent, Listener* listener) :
	wxPanel(parent, wxID_ANY),
	m_listener(listener) {

	InitLayout();

	// Pre-fill item list
	std::vector<uint16_t> ids;
	for (uint32_t i = 1; i <= static_cast<uint32_t>(g_items.getMaxID()); ++i) {
		const ItemType& it = g_items.getItemType(i);
		if (it.id != 0) {
			ids.push_back(i);
		}
	}
	m_allItemsGrid->SetItems(ids);

	PopulateBrushGrid();
}

LibraryPanel::~LibraryPanel() { }

void LibraryPanel::InitLayout() {
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	int padding = Theme::Grid(2);

	m_notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxBORDER_NONE);

	// PAGE 1: Item List
	wxPanel* itemListPage = new wxPanel(m_notebook);
	wxBoxSizer* itemListSizer = new wxBoxSizer(wxVERTICAL);

	m_itemSearch = new wxSearchCtrl(itemListPage, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);
	m_itemSearch->ShowCancelButton(true);
	m_itemSearch->SetBackgroundColour(Theme::Get(Theme::Role::Background));
	m_itemSearch->Bind(wxEVT_TEXT, &LibraryPanel::OnSearchChange, this);
	m_itemSearch->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &LibraryPanel::OnSearchChange, this);
	itemListSizer->Add(m_itemSearch, 0, wxEXPAND | wxALL, padding);

	m_allItemsGrid = new ItemGridPanel(itemListPage, this);
	m_allItemsGrid->SetDraggable(true);
	itemListSizer->Add(m_allItemsGrid, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, padding);

	itemListPage->SetSizer(itemListSizer);
	m_notebook->AddPage(itemListPage, "Items");

	// PAGE 2: Brush List
	wxPanel* brushListPage = new wxPanel(m_notebook);
	wxBoxSizer* brushListSizer = new wxBoxSizer(wxVERTICAL);
	wxSplitterWindow* brushSplitter = new wxSplitterWindow(brushListPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

	// Top: Brushes
	wxPanel* brushesPanel = new wxPanel(brushSplitter);
	wxBoxSizer* brushesSizer = new wxBoxSizer(wxVERTICAL);

	m_brushSearch = new wxSearchCtrl(brushesPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);
	m_brushSearch->ShowCancelButton(true);
	m_brushSearch->SetBackgroundColour(Theme::Get(Theme::Role::Background));
	m_brushSearch->Bind(wxEVT_TEXT, &LibraryPanel::OnBrushSearchChange, this);
	m_brushSearch->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &LibraryPanel::OnBrushSearchChange, this);
	brushesSizer->Add(m_brushSearch, 0, wxEXPAND | wxALL, 2);

	m_brushGrid = new ItemGridPanel(brushesPanel, this);
	m_brushGrid->SetShowDetails(false);
	brushesSizer->Add(new wxStaticText(brushesPanel, wxID_ANY, "Available Brushes"), 0, wxALL, 2);
	brushesSizer->Add(m_brushGrid, 1, wxEXPAND);
	brushesPanel->SetSizer(brushesSizer);

	// Bottom: Related Items
	wxPanel* relatedPanel = new wxPanel(brushSplitter);
	wxBoxSizer* relatedSizer = new wxBoxSizer(wxVERTICAL);
	m_relatedGrid = new ItemGridPanel(relatedPanel, this);
	m_relatedGrid->SetDraggable(true);
	relatedSizer->Add(new wxStaticText(relatedPanel, wxID_ANY, "Related Items"), 0, wxALL, 2);
	relatedSizer->Add(m_relatedGrid, 1, wxEXPAND);
	relatedPanel->SetSizer(relatedSizer);

	brushSplitter->SplitHorizontally(brushesPanel, relatedPanel);
	brushSplitter->SetSashGravity(0.5);
	brushListSizer->Add(brushSplitter, 1, wxEXPAND | wxALL, padding);

	brushListPage->SetSizer(brushListSizer);
	m_notebook->AddPage(brushListPage, "Brushes");

	mainSizer->Add(m_notebook, 1, wxEXPAND);
	SetSizer(mainSizer);
}

void LibraryPanel::OnItemSelected(ItemGridPanel* source, uint16_t itemId) {
	if (itemId == 0) {
		return;
	}

	if (source == m_brushGrid) {
		PopulateRelatedItems(itemId);
	} else if (m_listener) {
		m_listener->OnLibraryItemSelected(itemId);
	}
}

void LibraryPanel::OnSearchChange(wxCommandEvent&) {
	m_allItemsGrid->SetFilter(m_itemSearch->GetValue());
}

void LibraryPanel::OnBrushSearchChange(wxCommandEvent&) {
	PopulateBrushGrid();
}

uint16_t LibraryPanel::GetSidFromCid(uint16_t cid) {
	if (m_cidToSidCache.empty()) {
		uint32_t maxId = static_cast<uint32_t>(g_items.getMaxID());
		for (uint32_t id = 100; id <= maxId; ++id) {
			const ItemType& it = g_items.getItemType(id);
			if (it.id != 0 && it.clientID != 0) {
				if (m_cidToSidCache.find(it.clientID) == m_cidToSidCache.end()) {
					m_cidToSidCache[it.clientID] = it.id;
				}
			}
		}
	}
	auto it = m_cidToSidCache.find(cid);
	return (it != m_cidToSidCache.end()) ? it->second : 0;
}

void LibraryPanel::PopulateBrushGrid() {
	std::vector<uint16_t> brushIds;
	std::map<uint16_t, wxString> overrides;
	m_brushLookup.clear();

	wxString query = m_brushSearch->GetValue().Lower();

	for (const auto& pair : g_brushes.getMap()) {
		Brush* brush = pair.second.get();
		if (!brush || brush->isEraser() || brush->isRaw()) {
			continue;
		}

		std::string name = brush->getName();
		if (name.empty()) {
			continue;
		}

		if (!query.IsEmpty() && !wxString(name).Lower().Contains(query)) {
			continue;
		}

		int rawLookId = brush->getLookID();
		if (rawLookId < 0 || rawLookId > 0xFFFF) {
			continue;
		}
		uint16_t lookId = static_cast<uint16_t>(rawLookId);
		uint16_t serverId = GetSidFromCid(lookId);

		if (serverId != 0) {
			if (m_brushLookup.find(serverId) == m_brushLookup.end()) {
				brushIds.push_back(serverId);
				m_brushLookup[serverId] = brush;
				overrides[serverId] = name;
			}
		}
	}
	m_brushGrid->SetItems(brushIds);
	m_brushGrid->SetOverrideNames(overrides);
}

void LibraryPanel::PopulateRelatedItems(uint16_t brushLookId) {
	std::vector<uint16_t> related;
	auto it = m_brushLookup.find(brushLookId);
	if (it == m_brushLookup.end()) {
		m_relatedGrid->SetItems({});
		return;
	}

	Brush* brush = it->second;
	if (!brush) {
		return;
	}

	try {
		brush->getRelatedItems(related);
	} catch (const std::exception& e) {
		wxLogError("Error getting related items for brush %d: %s", brushLookId, e.what());
	} catch (...) {
		wxLogError("Unknown error getting related items for brush %d", brushLookId);
	}

	if (brushLookId != 0) {
		related.push_back(brushLookId);
	}

	std::sort(related.begin(), related.end());
	related.erase(std::unique(related.begin(), related.end()), related.end());

	m_relatedGrid->SetItems(related);
}
