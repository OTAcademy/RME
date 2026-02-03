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

	SetSize(FromDIP(wxSize(1200, 700))); // Wider for 4 columns
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

	Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &ReplaceToolWindow::OnSearchChange, this);
	Bind(wxEVT_TEXT, &ReplaceToolWindow::OnSearchChange, this, searchCtrl->GetId());
}

ReplaceToolWindow::~ReplaceToolWindow() { }

void ReplaceToolWindow::InitLayout() {
	wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

	int padding = Theme::Grid(2);
	wxFont headerFont = Theme::GetFont(8, true);
	wxColour subTextColor = Theme::Get(Theme::Role::TextSubtle);

	// ---------------------------------------------------------
	// COLUMN 1: Item Library
	// ---------------------------------------------------------
	wxBoxSizer* col1 = new wxBoxSizer(wxVERTICAL);
	col1->SetMinSize(FromDIP(240), -1);

	// Header
	wxStaticText* libTitle = new wxStaticText(this, wxID_ANY, "ITEM LIBRARY");
	libTitle->SetFont(headerFont);
	libTitle->SetForegroundColour(subTextColor);
	col1->Add(libTitle, 0, wxALL, padding);

	// Search
	searchCtrl = new wxSearchCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxBORDER_NONE);
	searchCtrl->ShowCancelButton(true);
	searchCtrl->SetBackgroundColour(Theme::Get(Theme::Role::Background));
	col1->Add(searchCtrl, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, padding);

	// Grid
	allItemsGrid = new ItemGridPanel(this, this);
	allItemsGrid->SetDraggable(true);
	col1->Add(allItemsGrid, 1, wxEXPAND | wxLEFT | wxRIGHT, padding);

	mainSizer->Add(col1, 3, wxEXPAND); // Flex 3

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

void ReplaceToolWindow::OnItemSelected(ItemGridPanel* source, uint16_t itemId) {
	// Only update suggestions if selected from the main library
	if (source == allItemsGrid) {
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
