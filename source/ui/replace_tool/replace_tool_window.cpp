#include "ui/replace_tool/replace_tool_window.h"
#include "ui/replace_tool/visual_similarity_service.h"
#include "ui/theme.h"
#include "editor/editor.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/srchctrl.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/artprov.h>

ReplaceToolWindow::ReplaceToolWindow(wxWindow* parent, Editor* editor) : wxDialog(parent, wxID_ANY, "Advanced Replace Tool", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
																		 editor(editor) {

	SetSize(FromDIP(wxSize(1400, 850))); // Wider for 64px columns
	VisualSimilarityService::Get().StartIndexing();
	SetBackgroundColour(Theme::Get(Theme::Role::Surface));

	InitLayout();

	// Pre-fill
	std::vector<uint16_t> ids;
	for (uint16_t i = 1; i <= g_items.getMaxID(); ++i) {
		const ItemType& it = g_items.getItemType(i);
		if (it.id != 0) {
			ids.push_back(i);
		}
	}
	allItemsGrid->SetItems(ids);

	UpdateSavedRulesList();

	Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &ReplaceToolWindow::OnSearchChange, this, searchCtrl->GetId());
	Bind(wxEVT_TEXT, &ReplaceToolWindow::OnSearchChange, this, searchCtrl->GetId());

	Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &ReplaceToolWindow::OnBrushSearchChange, this, brushSearchCtrl->GetId());
	Bind(wxEVT_TEXT, &ReplaceToolWindow::OnBrushSearchChange, this, brushSearchCtrl->GetId());
}

ReplaceToolWindow::~ReplaceToolWindow() { }

// Brush includes
#include "brushes/brush.h"
#include "brushes/ground/ground_brush.h"
#include "brushes/ground/auto_border.h"
#include "brushes/wall/wall_brush.h"
#include "brushes/doodad/doodad_brush.h"
#include "brushes/table/table_brush_items.h"
#include "brushes/carpet/carpet_brush_items.h"
#include "brushes/door/door_brush.h"
#include "brushes/table/table_brush.h"
#include "brushes/carpet/carpet_brush.h"
#include <wx/splitter.h>

void ReplaceToolWindow::InitLayout() {
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	int padding = Theme::Grid(2);
	wxFont headerFont = Theme::GetFont(8, true);
	wxColour subTextColor = Theme::Get(Theme::Role::TextSubtle);

	// ---------------------------------------------------------
	// COLUMN 1: Item Library (Tabs)
	// ---------------------------------------------------------
	wxBoxSizer* col1 = new wxBoxSizer(wxVERTICAL);
	col1->SetMinSize(FromDIP(240), -1);

	// Header
	wxStaticText* libTitle = new wxStaticText(this, wxID_ANY, "ITEM LIBRARY");
	libTitle->SetFont(headerFont);
	libTitle->SetForegroundColour(subTextColor);
	col1->Add(libTitle, 0, wxALL, padding);

	// Notebook for Tabs
	libraryTabs = new wxNotebook(this, wxID_ANY);

	// PAGE 1: Item List
	wxPanel* itemListPage = new wxPanel(libraryTabs);
	wxBoxSizer* itemListSizer = new wxBoxSizer(wxVERTICAL);

	searchCtrl = new wxSearchCtrl(itemListPage, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);
	searchCtrl->ShowCancelButton(true);
	searchCtrl->SetBackgroundColour(Theme::Get(Theme::Role::Background));
	itemListSizer->Add(searchCtrl, 0, wxEXPAND | wxALL, padding);

	allItemsGrid = new ItemGridPanel(itemListPage, this);
	allItemsGrid->SetDraggable(true);
	itemListSizer->Add(allItemsGrid, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, padding);

	itemListPage->SetSizer(itemListSizer);
	libraryTabs->AddPage(itemListPage, "Items");

	// PAGE 2: Brush List
	wxPanel* brushListPage = new wxPanel(libraryTabs);
	wxBoxSizer* brushListSizer = new wxBoxSizer(wxVERTICAL);
	wxSplitterWindow* brushSplitter = new wxSplitterWindow(brushListPage, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);

	// Top: Brushes
	wxPanel* brushesPanel = new wxPanel(brushSplitter);
	wxBoxSizer* brushesSizer = new wxBoxSizer(wxVERTICAL);

	brushSearchCtrl = new wxSearchCtrl(brushesPanel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);
	brushSearchCtrl->ShowCancelButton(true);
	brushSearchCtrl->SetBackgroundColour(Theme::Get(Theme::Role::Background));
	brushSearchCtrl->Bind(wxEVT_TEXT, &ReplaceToolWindow::OnBrushSearchChange, this);
	brushSearchCtrl->Bind(wxEVT_SEARCH, &ReplaceToolWindow::OnBrushSearchChange, this);
	brushesSizer->Add(brushSearchCtrl, 0, wxEXPAND | wxALL, 2);

	brushListGrid = new ItemGridPanel(brushesPanel, this); // Use same listener, handle via ID check
	brushListGrid->SetShowDetails(false);

	brushesSizer->Add(new wxStaticText(brushesPanel, wxID_ANY, "Available Brushes"), 0, wxALL, 2);
	brushesSizer->Add(brushListGrid, 1, wxEXPAND);
	brushesPanel->SetSizer(brushesSizer);

	// Bottom: Related Items
	wxPanel* relatedPanel = new wxPanel(brushSplitter);
	wxBoxSizer* relatedSizer = new wxBoxSizer(wxVERTICAL);
	brushRelatedGrid = new ItemGridPanel(relatedPanel, this);
	brushRelatedGrid->SetDraggable(true);

	relatedSizer->Add(new wxStaticText(relatedPanel, wxID_ANY, "Related Items"), 0, wxALL, 2);
	relatedSizer->Add(brushRelatedGrid, 1, wxEXPAND);
	relatedPanel->SetSizer(relatedSizer);

	brushSplitter->SplitHorizontally(brushesPanel, relatedPanel);
	brushSplitter->SetSashGravity(0.5);
	brushListSizer->Add(brushSplitter, 1, wxEXPAND | wxALL, padding);

	brushListPage->SetSizer(brushListSizer);
	libraryTabs->AddPage(brushListPage, "Brushes");

	col1->Add(libraryTabs, 1, wxEXPAND);
	mainSizer->Add(col1, 3, wxEXPAND); // Flex 3

	// PAGE 3: Similarity Grid
	wxPanel* similarPage = new wxPanel(this); // Added back if missing? Wait, was it a separate column?
	// It was actually in COLUMN 3 of the main sizer in previous design.

	// Populate Brushes
	PopulateBrushGrid();

	mainSizer->AddSpacer(padding);

	// ---------------------------------------------------------
	// COLUMN 2: Rule Builder
	// ---------------------------------------------------------
	wxBoxSizer* col2 = new wxBoxSizer(wxVERTICAL);
	col2->SetMinSize(FromDIP(300), -1);

	wxStaticText* builderTitle = new wxStaticText(this, wxID_ANY, "RULE BUILDER");
	builderTitle->SetFont(headerFont);
	builderTitle->SetForegroundColour(subTextColor);
	col2->Add(builderTitle, 0, wxALL, padding);

	ruleBuilder = new RuleBuilderPanel(this, this);
	col2->Add(ruleBuilder, 1, wxEXPAND | wxALL, padding);

	// Actions (Save, Execute)
	wxBoxSizer* actionSizer = new wxBoxSizer(wxHORIZONTAL);

	m_saveBtn = new wxButton(this, wxID_ANY, "Save Rule");
	m_saveBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnSaveRule, this);

	m_executeBtn = new wxButton(this, wxID_ANY, "Execute Replace");
	m_executeBtn->SetDefault();
	m_executeBtn->SetBackgroundColour(Theme::Get(Theme::Role::Accent));
	m_executeBtn->SetForegroundColour(*wxWHITE);
	m_executeBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnExecute, this);

	actionSizer->Add(m_saveBtn, 1, wxRIGHT, padding);
	actionSizer->Add(m_executeBtn, 1, wxLEFT, padding);

	col2->Add(actionSizer, 0, wxEXPAND | wxALL, padding);

	mainSizer->Add(col2, 4, wxEXPAND); // Flex 4

	// ---------------------------------------------------------
	// COLUMN 3: Similarity Engine
	// ---------------------------------------------------------
	wxBoxSizer* col3 = new wxBoxSizer(wxVERTICAL);
	col3->SetMinSize(FromDIP(240), -1);

	wxStaticText* simTitle = new wxStaticText(this, wxID_ANY, "SMART SUGGESTIONS");
	simTitle->SetFont(headerFont);
	simTitle->SetForegroundColour(subTextColor);
	col3->Add(simTitle, 0, wxALL, padding);

	similarItemsGrid = new ItemGridPanel(this, this);
	similarItemsGrid->SetDraggable(true);
	col3->Add(similarItemsGrid, 1, wxEXPAND | wxALL, padding);

	mainSizer->Add(col3, 3, wxEXPAND); // Flex 3

	// ---------------------------------------------------------
	// COLUMN 4: Rule Manager (Saved)
	// ---------------------------------------------------------
	wxBoxSizer* col4 = new wxBoxSizer(wxVERTICAL);
	col4->SetMinSize(FromDIP(200), -1);

	wxStaticText* savedTitle = new wxStaticText(this, wxID_ANY, "SAVED RULES");
	savedTitle->SetFont(headerFont);
	savedTitle->SetForegroundColour(subTextColor);
	col4->Add(savedTitle, 0, wxALL, padding);

	savedRulesList = new RuleListControl(this, this);
	col4->Add(savedRulesList, 1, wxEXPAND | wxALL, padding);

	mainSizer->Add(col4, 2, wxEXPAND); // Flex 2

	SetSizer(mainSizer);
	Layout();
}

uint16_t ReplaceToolWindow::GetSidFromCid(uint16_t cid) {
	if (cidToSidCache.empty()) {
		uint16_t maxId = g_items.getMaxID();
		for (uint16_t id = 100; id <= maxId; ++id) {
			const ItemType& it = g_items.getItemType(id);
			if (it.id != 0 && it.clientID != 0) {
				if (cidToSidCache.find(it.clientID) == cidToSidCache.end()) {
					cidToSidCache[it.clientID] = it.id;
				}
			}
		}
	}
	auto it = cidToSidCache.find(cid);
	return (it != cidToSidCache.end()) ? it->second : 0;
}

void ReplaceToolWindow::PopulateBrushGrid() {
	std::vector<uint16_t> brushIds;
	std::map<uint16_t, wxString> overrides;
	brushLookup.clear();

	wxString query = brushSearchCtrl->GetValue().Lower();

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

		uint16_t lookId = static_cast<uint16_t>(brush->getLookID());
		uint16_t serverId = GetSidFromCid(lookId);

		if (serverId != 0) {
			if (brushLookup.find(serverId) == brushLookup.end()) {
				brushIds.push_back(serverId);
				brushLookup[serverId] = brush;
				overrides[serverId] = name;
			}
		}
	}
	brushListGrid->SetItems(brushIds);
	brushListGrid->SetOverrideNames(overrides);
}

void ReplaceToolWindow::OnBrushSearchChange(wxCommandEvent&) {
	PopulateBrushGrid();
}

void ReplaceToolWindow::PopulateRelatedItems(uint16_t brushLookId) {
	std::vector<uint16_t> related;
	auto it = brushLookup.find(brushLookId);
	if (it == brushLookup.end()) {
		brushRelatedGrid->SetItems({});
		return;
	}

	Brush* brush = it->second;
	if (!brush) {
		return;
	}

	try {
		if (GroundBrush* gb = brush->asGround()) {
			for (const auto* block : gb->borders) {
				if (block && block->autoborder) {
					for (uint32_t tileId : block->autoborder->tiles) {
						if (tileId != 0 && g_items.typeExists(tileId)) {
							related.push_back(tileId);
						}
					}
				}
			}
		} else if (WallBrush* wb = brush->asWall()) {
			for (int i = 0; i <= 16; ++i) {
				const auto& node = wb->items.getWallNode(i);
				for (const auto& item : node.items) {
					if (item.id != 0) {
						related.push_back(item.id);
					}
				}
				const auto& doors = wb->items.getDoorItems(i);
				for (const auto& door : doors) {
					if (door.id != 0) {
						related.push_back(door.id);
					}
				}
			}
		} else if (DoodadBrush* db = brush->asDoodad()) {
			for (const auto& alt : db->items.getAlternatives()) {
				if (!alt) {
					continue;
				}
				for (const auto& single : alt->single_items) {
					if (single.item && single.item->getID() != 0) {
						related.push_back(single.item->getID());
					}
				}
				for (const auto& composite : alt->composite_items) {
					for (const auto& entry : composite.items) {
						for (const auto& item : entry.second) {
							if (item && item->getID() != 0) {
								related.push_back(item->getID());
							}
						}
					}
				}
			}
		} else if (TableBrush* tb = brush->asTable()) {
			for (int i = 0; i < 7; ++i) {
				if (tb->items.hasItems(i)) {
					const auto& node = tb->items.getItems(i);
					for (const auto& t : node.items) {
						if (t.item_id != 0) {
							related.push_back(t.item_id);
						}
					}
				}
			}
		} else if (CarpetBrush* cb = brush->asCarpet()) {
			for (const auto& group : cb->m_items.m_groups) {
				for (const auto& item : group.items) {
					if (item.id != 0) {
						related.push_back(item.id);
					}
				}
			}
		}
	} catch (...) { }

	if (brushLookId != 0) {
		related.push_back(brushLookId);
	}

	std::sort(related.begin(), related.end());
	related.erase(std::unique(related.begin(), related.end()), related.end());

	brushRelatedGrid->SetItems(related);
}

void ReplaceToolWindow::OnItemSelected(ItemGridPanel* source, uint16_t itemId) {
	if (itemId == 0) {
		return;
	}

	if (source == allItemsGrid) {
		similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(itemId));
	} else if (source == brushListGrid) {
		PopulateRelatedItems(itemId);
	} else if (source == brushRelatedGrid) {
		similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(itemId));
	}
}

void ReplaceToolWindow::OnRuleSelected(const RuleSet& rs) {
	if (!rs.rules.empty()) {
		ruleBuilder->SetRules(rs.rules);
		similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(rs.rules[0].fromId));
	}
}

void ReplaceToolWindow::OnRuleChanged() { }

void ReplaceToolWindow::OnSearchChange(wxCommandEvent&) {
	allItemsGrid->SetFilter(searchCtrl->GetValue());
}

void ReplaceToolWindow::OnExecute(wxCommandEvent&) {
	std::vector<ReplacementRule> rules = ruleBuilder->GetRules();
	if (rules.empty()) {
		wxMessageBox("Please create at least one rule.", "Error", wxICON_WARNING);
		return;
	}

	bool selectionOnly = editor->selection.size() > 0;
	std::map<uint16_t, const ReplacementRule*> ruleMap;
	for (const auto& rule : rules) {
		if (rule.fromId != 0) {
			ruleMap[rule.fromId] = &rule;
		}
	}

	auto finder = [&](Map&, Tile*, Item* item, long long) {
		auto it = ruleMap.find(item->getID());
		if (it != ruleMap.end()) {
			uint16_t newId;
			if (engine.ResolveReplacement(newId, *it->second)) {
				item->setID(newId);
			}
		}
	};

	foreach_ItemOnMap(editor->map, finder, selectionOnly);
	editor->map.doChange();
	g_gui.RefreshView();
}

void ReplaceToolWindow::OnSaveRule(wxCommandEvent&) {
	std::vector<ReplacementRule> rules = ruleBuilder->GetRules();
	if (rules.empty()) {
		return;
	}

	wxString name = wxGetTextFromUser("Enter name for this rule set:", "Save Rule", "");
	if (name.IsEmpty()) {
		return;
	}

	RuleSet rs;
	rs.name = name.ToStdString();
	rs.rules = rules;

	if (RuleManager::Get().SaveRuleSet(rs)) {
		UpdateSavedRulesList();
	}
}

void ReplaceToolWindow::UpdateSavedRulesList() {
	savedRulesList->SetRuleSets(RuleManager::Get().GetAvailableRuleSets());
}

void ReplaceToolWindow::OnClearSource(wxCommandEvent&) {
	ruleBuilder->Clear();
	similarItemsGrid->SetItems({});
}
