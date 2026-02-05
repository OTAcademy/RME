#include "ui/replace_tool/replace_tool_window.h"
#include "ui/replace_tool/visual_similarity_service.h"
#include "ui/theme.h"
#include "editor/editor.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include <algorithm> // For std::find
#include <wx/statline.h>
#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/srchctrl.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/artprov.h>

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

// Include at top
#include "ui/replace_tool/card_panel.h"

void ReplaceToolWindow::InitLayout() {
	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);

	int padding = Theme::Grid(2);
	wxFont headerFont = Theme::GetFont(9, true); // Slightly larger
	wxColour subTextColor = Theme::Get(Theme::Role::TextSubtle);

	// ---------------------------------------------------------
	// MAIN CONTENT ROW
	// ---------------------------------------------------------
	wxBoxSizer* mainRowSizer = new wxBoxSizer(wxHORIZONTAL);

	// ---------------------------------------------------------
	// COLUMN 1: Item Library
	// ---------------------------------------------------------
	CardPanel* col1Card = new CardPanel(this, wxID_ANY);
	col1Card->SetTitle("ITEM LIBRARY");

	// Notebook for Tabs
	libraryTabs = new wxNotebook(col1Card, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP | wxBORDER_NONE);

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

	brushListGrid = new ItemGridPanel(brushesPanel, this);
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

	col1Card->GetContentSizer()->Add(libraryTabs, 1, wxEXPAND | wxALL, padding / 2); // Less padding inside card
	mainRowSizer->Add(col1Card, 3, wxEXPAND | wxLEFT | wxRIGHT, padding / 2); // Flex 3

	// ---------------------------------------------------------
	// COLUMN 2: Rule Builder
	// ---------------------------------------------------------
	CardPanel* col2Card = new CardPanel(this, wxID_ANY);
	col2Card->SetTitle("RULE BUILDER");

	ruleBuilder = new RuleBuilderPanel(col2Card, this);
	col2Card->GetContentSizer()->Add(ruleBuilder, 1, wxEXPAND | wxALL, padding);
	col2Card->SetShowFooter(true);

	// Rule Builder Footer Buttons
	wxBoxSizer* ruleFooterSizer = new wxBoxSizer(wxHORIZONTAL);
	m_saveBtn = new wxButton(col2Card, wxID_ANY, "Save Rule");
	m_executeBtn = new wxButton(col2Card, wxID_ANY, "Execute Replace");
	m_executeBtn->SetDefault();
	m_executeBtn->SetBackgroundColour(Theme::Get(Theme::Role::Accent));
	m_executeBtn->SetForegroundColour(*wxWHITE);

	ruleFooterSizer->AddStretchSpacer(1);
	ruleFooterSizer->Add(m_saveBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, padding / 2);
	ruleFooterSizer->Add(m_executeBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, padding / 2);
	ruleFooterSizer->AddStretchSpacer(1);
	col2Card->GetFooterSizer()->Add(ruleFooterSizer, 1, wxEXPAND);

	m_saveBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnSaveRule, this);
	m_executeBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnExecute, this);

	mainRowSizer->Add(col2Card, 4, wxEXPAND | wxLEFT | wxRIGHT, padding / 2); // Flex 4

	// ---------------------------------------------------------
	// COLUMN 3: Similarity Engine
	// ---------------------------------------------------------
	CardPanel* col3Card = new CardPanel(this, wxID_ANY);
	col3Card->SetTitle("SMART SUGGESTIONS");

	similarItemsGrid = new ItemGridPanel(col3Card, this);
	similarItemsGrid->SetDraggable(true);
	col3Card->GetContentSizer()->Add(similarItemsGrid, 1, wxEXPAND | wxALL, padding);
	mainRowSizer->Add(col3Card, 3, wxEXPAND | wxLEFT | wxRIGHT, padding / 2); // Flex 3

	// ---------------------------------------------------------
	// COLUMN 4: Saved Rules
	// ---------------------------------------------------------
	CardPanel* col4Card = new CardPanel(this, wxID_ANY);
	col4Card->SetTitle("SAVED RULES");
	col4Card->SetShowFooter(true);

	savedRulesList = new RuleListControl(col4Card, this);
	col4Card->GetContentSizer()->Add(savedRulesList, 1, wxEXPAND | wxALL, padding);

	// Saved Rules Footer Buttons
	wxBoxSizer* savedFooterSizer = new wxBoxSizer(wxHORIZONTAL);
	m_addRuleBtn = new wxButton(col4Card, wxID_ANY, "Add", wxDefaultPosition, FromDIP(wxSize(60, -1)));
	m_editRuleBtn = new wxButton(col4Card, wxID_ANY, "Edit", wxDefaultPosition, FromDIP(wxSize(60, -1)));
	m_deleteRuleBtn = new wxButton(col4Card, wxID_ANY, "Del", wxDefaultPosition, FromDIP(wxSize(60, -1)));

	savedFooterSizer->AddStretchSpacer(1);
	savedFooterSizer->Add(m_addRuleBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
	savedFooterSizer->Add(m_editRuleBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
	savedFooterSizer->Add(m_deleteRuleBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);
	savedFooterSizer->AddStretchSpacer(1);
	col4Card->GetFooterSizer()->Add(savedFooterSizer, 1, wxEXPAND);

	m_addRuleBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
		std::vector<ReplacementRule> rules = ruleBuilder->GetRules();
		if (rules.empty()) {
			return;
		}
		wxString name = wxGetTextFromUser("Enter name for this rule set:", "Add Rule", "");
		if (!name.IsEmpty()) {
			RuleSet rs;
			rs.name = name.ToStdString();
			rs.rules = rules;
			if (RuleManager::Get().SaveRuleSet(rs)) {
				UpdateSavedRulesList();
			}
		}
	});

	m_editRuleBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
		if (savedRulesList->GetSelectedIndex() != -1) {
			std::string oldName = savedRulesList->GetRuleSetNames()[savedRulesList->GetSelectedIndex()];
			wxString newName = wxGetTextFromUser("Enter new name for rule set:", "Edit Name", oldName);
			if (!newName.IsEmpty() && newName.ToStdString() != oldName) {
				OnRuleRenamed(oldName, newName.ToStdString());
			}
		}
	});

	m_deleteRuleBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) {
		if (savedRulesList->GetSelectedIndex() != -1) {
			std::string name = savedRulesList->GetRuleSetNames()[savedRulesList->GetSelectedIndex()];
			wxMessageDialog dlg(this, "Are you sure you want to delete rule set '" + name + "'?", "Delete Rule Set", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
			if (dlg.ShowModal() == wxID_YES) {
				OnRuleDeleted(name);
			}
		}
	});

	mainRowSizer->Add(col4Card, 2, wxEXPAND | wxLEFT | wxRIGHT, padding / 2); // Flex 2

	// Add Main Row to Root
	rootSizer->Add(mainRowSizer, 1, wxEXPAND | wxALL, padding / 2);

	// Remove Global FooterCard
	/*
	CardPanel* footerCard = new CardPanel(this, wxID_ANY);
	...
	*/

	// Populate
	PopulateBrushGrid();

	SetSizer(rootSizer);
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
			std::vector<uint16_t> items;
			gb->getRelatedItems(items);
			for (uint16_t id : items) {
				if (id != 0 && g_items.typeExists(id)) {
					related.push_back(id);
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
	} catch (const std::exception& e) {
		wxLogError("Error populating related items: %s", e.what());
	} catch (...) {
		wxLogError("Unknown error populating related items.");
	}

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
	m_activeRuleSetName = rs.name;
	ruleBuilder->SetRules(rs.rules);
	if (!rs.rules.empty()) {
		similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(rs.rules[0].fromId));
	} else {
		similarItemsGrid->SetItems({});
	}
}

void ReplaceToolWindow::OnRuleDeleted(const std::string& name) {
	if (RuleManager::Get().DeleteRuleSet(name)) {
		UpdateSavedRulesList();
	}
}

void ReplaceToolWindow::OnRuleRenamed(const std::string& oldName, const std::string& newName) {
	std::vector<std::string> existing = RuleManager::Get().GetAvailableRuleSets();
	if (std::find(existing.begin(), existing.end(), newName) != existing.end()) {
		wxMessageBox("A rule set with this name already exists.", "Rename Error", wxOK | wxICON_ERROR);
		return;
	}

	RuleSet rs = RuleManager::Get().LoadRuleSet(oldName);
	if (!rs.name.empty()) {
		RuleManager::Get().DeleteRuleSet(oldName);
		rs.name = newName;
		RuleManager::Get().SaveRuleSet(rs);

		if (m_activeRuleSetName == oldName) {
			m_activeRuleSetName = newName;
		}

		UpdateSavedRulesList();
	}
}

void ReplaceToolWindow::OnRuleChanged() {
	if (!m_activeRuleSetName.empty()) {
		RuleSet rs;
		rs.name = m_activeRuleSetName;
		rs.rules = ruleBuilder->GetRules();
		RuleManager::Get().SaveRuleSet(rs);
	}
}

void ReplaceToolWindow::OnSearchChange(wxCommandEvent&) {
	allItemsGrid->SetFilter(searchCtrl->GetValue());
}

void ReplaceToolWindow::OnExecute(wxCommandEvent&) {
	std::vector<ReplacementRule> rules = ruleBuilder->GetRules();
	if (rules.empty()) {
		return;
	}

	static const uint16_t TRASH_ITEM_ID = 0xFFFF; // Must match RuleBuilderPanel

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
				if (newId == TRASH_ITEM_ID) {
					item->setID(0); // Delete/Clear item
				} else {
					item->setID(newId);
				}
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

	std::vector<std::string> existing = RuleManager::Get().GetAvailableRuleSets();
	if (std::find(existing.begin(), existing.end(), name.ToStdString()) != existing.end()) {
		wxMessageBox("A rule set with this name already exists. Please choose a different name.", "Name Collision", wxOK | wxICON_ERROR);
		return;
	}

	RuleSet rs;
	rs.name = name.ToStdString();
	rs.rules = rules;

	if (RuleManager::Get().SaveRuleSet(rs)) {
		m_activeRuleSetName = rs.name;
		UpdateSavedRulesList();
	}
}

void ReplaceToolWindow::UpdateSavedRulesList() {
	savedRulesList->SetRuleSets(RuleManager::Get().GetAvailableRuleSets());
}

void ReplaceToolWindow::OnClearRules() {
	// Any additional logic when rules are cleared?
	// The rule builder already cleared itself.
	ruleBuilder->Clear();
	similarItemsGrid->SetItems({});
}
