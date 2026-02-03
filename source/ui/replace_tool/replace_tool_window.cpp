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
	// We might need to differentiate ID selection source

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

	// Populate Brushes
	PopulateBrushGrid();

	// Separator
	// User said "No visible grid lines... Use spacing". But columns usually need some separation.
	// I'll use a 1px transparent spacer or very subtle line if needed.
	// User said "Do NOT use visible grid lines or hard separators". I'll use simple spacing.
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

	mainSizer->AddSpacer(padding);

	// ---------------------------------------------------------
	// COLUMN 3: Suggestions
	// ---------------------------------------------------------
	wxBoxSizer* col3 = new wxBoxSizer(wxVERTICAL);
	col3->SetMinSize(FromDIP(200), -1);

	wxStaticText* suggestTitle = new wxStaticText(this, wxID_ANY, "SUGGESTIONS");
	suggestTitle->SetFont(headerFont);
	suggestTitle->SetForegroundColour(subTextColor);
	col3->Add(suggestTitle, 0, wxALL, padding);

	similarItemsGrid = new ItemGridPanel(this, this); // Re-use listener? Or NULL listener if we don't want selection in suggestions to update suggestions?
	// If I select logic in suggestions, maybe it should update suggestions? Or just draggable?
	// Let's keep listener 'this' so clicking a suggestion selects it and maybe updates suggestions (recursive lookalike finding).
	similarItemsGrid->SetDraggable(true);
	col3->Add(similarItemsGrid, 1, wxEXPAND | wxLEFT | wxRIGHT, padding);

	mainSizer->Add(col3, 2, wxEXPAND); // Flex 2

	mainSizer->AddSpacer(padding);

	// ---------------------------------------------------------
	// COLUMN 4: Ruleset Management
	// ---------------------------------------------------------
	wxBoxSizer* col4 = new wxBoxSizer(wxVERTICAL);
	col4->SetMinSize(FromDIP(200), -1);

	wxStaticText* rulesTitle = new wxStaticText(this, wxID_ANY, "SAVED RULES");
	rulesTitle->SetFont(headerFont);
	rulesTitle->SetForegroundColour(subTextColor);
	col4->Add(rulesTitle, 0, wxALL, padding);

	savedRulesList = new RuleListControl(this, this);
	col4->Add(savedRulesList, 1, wxEXPAND | wxLEFT | wxRIGHT, padding);

	// Administrative actions
	wxBoxSizer* adminSizer = new wxBoxSizer(wxHORIZONTAL);
	// Add small buttons or just rely on Right Click in list?
	// User: "Actions: Create new, Rename, Delete... Actions are subtle and secondary"
	// I'll add them as small text buttons or bitmap buttons at bottom

	// For now, simple textual buttons or placeholders if RuleControl handles context menu.
	// I will assume context menu or future implementation for Rename/Delete to verify design first.
	// But "Create new" matches "Save Rule" in col 2 effectively.

	mainSizer->Add(col4, 2, wxEXPAND); // Flex 2

	SetSizer(mainSizer);
	Layout();
}

uint16_t ReplaceToolWindow::GetSidFromCid(uint16_t cid) {
	if (cidToSidCache.empty()) {
		// Populate cache
		uint16_t maxId = g_items.getMaxID();
		for (uint16_t id = 100; id <= maxId; ++id) {
			const ItemType& it = g_items.getItemType(id);
			if (it.id != 0 && it.clientID != 0) {
				// Store first mapping found, or prefer items with valid names?
				// Just overwriting is fine, but maybe check if exists?
				if (cidToSidCache.find(it.clientID) == cidToSidCache.end()) {
					cidToSidCache[it.clientID] = it.id;
				}
			}
		}
	}
	auto it = cidToSidCache.find(cid);
	if (it != cidToSidCache.end()) {
		return it->second;
	}
	return 0;
}

void ReplaceToolWindow::PopulateBrushGrid() {
	std::vector<uint16_t> brushIds;
	std::map<uint16_t, wxString> overrides;
	brushLookup.clear();

	wxString query = brushSearchCtrl->GetValue().Lower();

	// We need to iterate brushes from Brushes which is g_brushes
	for (const auto& pair : g_brushes.getMap()) {
		Brush* brush = pair.second.get();
		if (!brush) {
			continue;
		}

		// Filter irrelevant brushes if needed (like Eraser)
		if (brush->isEraser() || brush->isRaw()) {
			continue;
		}

		// Required: Named brushes only
		std::string name = brush->getName();
		if (name.empty()) {
			continue;
		}

		// Search Filter
		if (!query.IsEmpty()) {
			wxString wName(name);
			if (!wName.Lower().Contains(query)) {
				continue;
			}
		}

		// Resolve Look ID (which is likely a Client ID) to a Server ID
		uint16_t lookId = static_cast<uint16_t>(brush->getLookID());
		uint16_t serverId = 0;
		if (lookId != 0) {
			serverId = GetSidFromCid(lookId);
		}

		if (serverId != 0) {
			// Avoid duplicates if multiple brushes share look ID?
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

	if (GroundBrush* gb = brush->asGround()) {
		// Ground Borders
		for (const auto* block : gb->borders) {
			if (block->autoborder) {
				for (uint32_t tileId : block->autoborder->tiles) {
					if (tileId != 0 && g_items.typeExists(tileId)) {
						related.push_back(tileId);
					}
				}
			}
		}
	} else if (WallBrush* wb = brush->asWall()) {
		// Walls
		for (int i = 0; i <= 16; ++i) {
			try {
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
			} catch (...) { }
		}
	} else if (DoodadBrush* db = brush->asDoodad()) {
		// Doodad
		for (const auto& alt : db->items.getAlternatives()) {
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
		// Table
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
		// Carpet
		for (const auto& group : cb->m_items.m_groups) {
			for (const auto& item : group.items) {
				if (item.id != 0) {
					related.push_back(item.id);
				}
			}
		}
	} else if (DoorBrush* doorb = brush->asDoor()) {
		// Door
		// Usually handled by generic inclusion below, but assuming ID match
	}

	// Add the look ID itself (which is the server ID passed in)
	if (brushLookId != 0) {
		related.push_back(brushLookId);
	}

	// Unique and Sort
	std::sort(related.begin(), related.end());
	related.erase(std::unique(related.begin(), related.end()), related.end());

	brushRelatedGrid->SetItems(related);
}

void ReplaceToolWindow::OnItemSelected(ItemGridPanel* source, uint16_t itemId) {
	if (source == allItemsGrid) { // Main Item Library
		similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(itemId));
	} else if (source == brushListGrid) { // Brush List
		// User selected a brush (represented by look ID)
		PopulateRelatedItems(itemId);
		// Also update suggestions? Maybe not.
	} else if (source == brushRelatedGrid) { // Related Items List
		// Selecting a related item. Behave like selecting from main library?
		similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(itemId));
	}
}

void ReplaceToolWindow::OnRuleSelected(const RuleSet& rs) {
	if (!rs.rules.empty()) {
		ruleBuilder->SetRules(rs.rules);
		// Update suggestions based on source of *first* rule
		similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(rs.rules[0].fromId));
	}
}

void ReplaceToolWindow::OnRuleChanged() {
	// Enable/disable buttons?
	// For now just refresh if needed logic
}

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

	// Build a lookup for fast access
	// Map fromId -> Rule
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
