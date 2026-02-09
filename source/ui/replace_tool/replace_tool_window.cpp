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
#include "ui/replace_tool/card_panel.h"
#include <wx/splitter.h>

ReplaceToolWindow::ReplaceToolWindow(wxWindow* parent, Editor* editor) : wxDialog(parent, wxID_ANY, "Advanced Replace Tool", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
																		 editor(editor) {

	SetSize(FromDIP(wxSize(1400, 850))); // Wider for 64px columns
	VisualSimilarityService::Get().StartIndexing();
	SetBackgroundColour(Theme::Get(Theme::Role::Surface));

	InitLayout();

	UpdateSavedRulesList();
}

ReplaceToolWindow::~ReplaceToolWindow() { }

void ReplaceToolWindow::InitLayout() {
	wxBoxSizer* rootSizer = new wxBoxSizer(wxVERTICAL);
	int padding = Theme::Grid(2);

	// ---------------------------------------------------------
	// MAIN CONTENT ROW
	// ---------------------------------------------------------
	wxBoxSizer* mainRowSizer = new wxBoxSizer(wxHORIZONTAL);

	// ---------------------------------------------------------
	// COLUMN 1: Library Panel (Items & Brushes)
	// ---------------------------------------------------------
	CardPanel* col1Card = new CardPanel(this, wxID_ANY);
	col1Card->SetTitle("ITEM LIBRARY");

	libraryPanel = new LibraryPanel(col1Card, this);
	col1Card->GetContentSizer()->Add(libraryPanel, wxSizerFlags(1).Expand().Border(wxALL, padding / 2));
	mainRowSizer->Add(col1Card, wxSizerFlags(3).Expand().Border(wxLEFT | wxRIGHT, padding / 2)); // Flex 3

	// ---------------------------------------------------------
	// COLUMN 2: Rule Builder
	// ---------------------------------------------------------
	CardPanel* col2Card = new CardPanel(this, wxID_ANY);
	col2Card->SetTitle("RULE BUILDER");

	ruleBuilder = new RuleBuilderPanel(col2Card, this);
	col2Card->GetContentSizer()->Add(ruleBuilder, wxSizerFlags(1).Expand().Border(wxALL, padding));
	col2Card->SetShowFooter(true);

	// Rule Builder Footer Buttons
	wxBoxSizer* ruleFooterSizer = new wxBoxSizer(wxHORIZONTAL);
	m_saveBtn = new wxButton(col2Card, wxID_ANY, "Save Rule");
	m_executeBtn = new wxButton(col2Card, wxID_ANY, "Execute Replace");
	m_executeBtn->SetDefault();
	m_executeBtn->SetBackgroundColour(Theme::Get(Theme::Role::Accent));
	m_executeBtn->SetForegroundColour(*wxWHITE);

	ruleFooterSizer->AddStretchSpacer(1);
	ruleFooterSizer->Add(m_saveBtn, wxSizerFlags(0).Border(wxALL | wxALIGN_CENTER_VERTICAL, padding / 2));
	ruleFooterSizer->Add(m_executeBtn, wxSizerFlags(0).Border(wxALL | wxALIGN_CENTER_VERTICAL, padding / 2));
	ruleFooterSizer->AddStretchSpacer(1);
	col2Card->GetFooterSizer()->Add(ruleFooterSizer, wxSizerFlags(1).Expand());

	m_saveBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnSaveRule, this);
	m_executeBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnExecute, this);

	mainRowSizer->Add(col2Card, wxSizerFlags(4).Expand().Border(wxLEFT | wxRIGHT, padding / 2)); // Flex 4

	// ---------------------------------------------------------
	// COLUMN 3: Similarity Engine
	// ---------------------------------------------------------
	CardPanel* col3Card = new CardPanel(this, wxID_ANY);
	col3Card->SetTitle("SMART SUGGESTIONS");

	similarItemsGrid = new ItemGridPanel(col3Card, this);
	similarItemsGrid->SetDraggable(true);
	col3Card->GetContentSizer()->Add(similarItemsGrid, wxSizerFlags(1).Expand().Border(wxALL, padding));
	mainRowSizer->Add(col3Card, wxSizerFlags(3).Expand().Border(wxLEFT | wxRIGHT, padding / 2)); // Flex 3

	// ---------------------------------------------------------
	// COLUMN 4: Saved Rules
	// ---------------------------------------------------------
	CardPanel* col4Card = new CardPanel(this, wxID_ANY);
	col4Card->SetTitle("SAVED RULES");
	col4Card->SetShowFooter(true);

	savedRulesList = new RuleListControl(col4Card, this);
	col4Card->GetContentSizer()->Add(savedRulesList, wxSizerFlags(1).Expand().Border(wxALL, padding));

	// Saved Rules Footer Buttons
	wxBoxSizer* savedFooterSizer = new wxBoxSizer(wxHORIZONTAL);
	m_addRuleBtn = new wxButton(col4Card, wxID_ANY, "Add", wxDefaultPosition, FromDIP(wxSize(60, -1)));
	m_editRuleBtn = new wxButton(col4Card, wxID_ANY, "Edit", wxDefaultPosition, FromDIP(wxSize(60, -1)));
	m_deleteRuleBtn = new wxButton(col4Card, wxID_ANY, "Del", wxDefaultPosition, FromDIP(wxSize(60, -1)));

	savedFooterSizer->AddStretchSpacer(1);
	savedFooterSizer->Add(m_addRuleBtn, wxSizerFlags(0).Border(wxALL | wxALIGN_CENTER_VERTICAL, 2));
	savedFooterSizer->Add(m_editRuleBtn, wxSizerFlags(0).Border(wxALL | wxALIGN_CENTER_VERTICAL, 2));
	savedFooterSizer->Add(m_deleteRuleBtn, wxSizerFlags(0).Border(wxALL | wxALIGN_CENTER_VERTICAL, 2));
	savedFooterSizer->AddStretchSpacer(1);
	col4Card->GetFooterSizer()->Add(savedFooterSizer, wxSizerFlags(1).Expand());

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

	mainRowSizer->Add(col4Card, wxSizerFlags(2).Expand().Border(wxLEFT | wxRIGHT, padding / 2)); // Flex 2

	// Add Main Row to Root
	rootSizer->Add(mainRowSizer, wxSizerFlags(1).Expand().Border(wxALL, padding / 2));

	SetSizer(rootSizer);
	Layout();
}

void ReplaceToolWindow::OnLibraryItemSelected(uint16_t itemId) {
	if (itemId == 0) {
		return;
	}
	similarItemsGrid->SetItems(VisualSimilarityService::Get().FindSimilar(itemId));
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
		rs.name = newName;
		if (RuleManager::Get().SaveRuleSet(rs)) {
			RuleManager::Get().DeleteRuleSet(oldName);

			if (m_activeRuleSetName == oldName) {
				m_activeRuleSetName = newName;
			}
			UpdateSavedRulesList();
		} else {
			wxMessageBox("Failed to save the renamed rule set.", "Rename Error", wxOK | wxICON_ERROR);
		}
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

void ReplaceToolWindow::OnExecute(wxCommandEvent&) {
	std::vector<ReplacementRule> rules = ruleBuilder->GetRules();
	if (rules.empty()) {
		return;
	}

	int confirm = wxMessageBox(
		"This will perform a bulk replacement across the entire map. This action cannot be easily undone.\n\nAre you sure you want to proceed?",
		"Confirm Bulk Replacement",
		wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
	);

	if (confirm == wxYES) {
		engine.ExecuteReplacement(editor, rules);
	}
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
	ruleBuilder->Clear();
	similarItemsGrid->SetItems({});
}

void ReplaceToolWindow::OnItemSelected(ItemGridPanel* source, uint16_t itemId) {
	if (source == similarItemsGrid) {
		OnLibraryItemSelected(itemId);
	}
}
