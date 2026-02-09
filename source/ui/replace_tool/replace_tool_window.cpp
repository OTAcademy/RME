#include "app/main.h"
#include "replace_tool_window.h"
#include "visual_similarity_service.h"
#include "ui/theme.h"
#include "editor/editor.h"
#include "game/items.h"
#include "ui/gui.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "app/managers/version_manager.h"
#include <algorithm> // For std::find
#include "ui/map_window.h"
#include "rendering/ui/map_display.h"
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

	VisualSimilarityService::Get().StartIndexing();
	SetBackgroundColour(Theme::Get(Theme::Role::Surface));

	InitLayout();

	UpdateSavedRulesList();

	Bind(wxEVT_CLOSE_WINDOW, &ReplaceToolWindow::OnClose, this);

	// Restore window size (Physical pixels)
	int w = g_settings.getInteger(Config::REPLACE_TOOL_WINDOW_WIDTH);
	int h = g_settings.getInteger(Config::REPLACE_TOOL_WINDOW_HEIGHT);

	if (w < 600 || h < 400) {
		// Default size (Logical -> Physical)
		SetSize(FromDIP(wxSize(1400, 850)));
	} else {
		// Restored size (Already Physical)
		SetSize(wxSize(w, h));
	}
}

ReplaceToolWindow::~ReplaceToolWindow() {
	// Unbind to be safe, though destructor usually handles cleanup
	Unbind(wxEVT_CLOSE_WINDOW, &ReplaceToolWindow::OnClose, this);
}

void ReplaceToolWindow::OnClose(wxCloseEvent& event) {
	if (!IsMaximized()) {
		// GetSize returns Physical pixels on Windows
		wxSize size = GetSize();
		g_settings.setInteger(Config::REPLACE_TOOL_WINDOW_WIDTH, size.GetWidth());
		g_settings.setInteger(Config::REPLACE_TOOL_WINDOW_HEIGHT, size.GetHeight());
		g_settings.save();
	}
	event.Skip(); // Allow window to close
}

void ReplaceToolWindow::InitializeWithIDs(const std::vector<uint16_t>& ids) {
	if (ids.empty()) {
		return;
	}

	std::vector<ReplacementRule> rules;
	for (uint16_t id : ids) {
		ReplacementRule rule;
		rule.fromId = id;
		rules.push_back(rule);
	}

	// IDs are already sorted from std::set in MapMenuHandler, but sorting here ensures consistency
	std::sort(rules.begin(), rules.end(), [](const ReplacementRule& a, const ReplacementRule& b) {
		return a.fromId < b.fromId;
	});

	ruleBuilder->SetRules(rules);
	m_activeRuleSetName = "";
}

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
	col2Card->SetShowFooter(false); // No footer anymore

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
	// COLUMN 4: Saved Rules (Split: List / Actions)
	// ---------------------------------------------------------
	// We use a splitter, but instead of raw panels, we put CardPanels inside to get the headers
	wxSplitterWindow* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH | wxSP_NOBORDER);

	// -- Top Card: Rules List --
	CardPanel* savedRulesCard = new CardPanel(splitter, wxID_ANY);
	savedRulesCard->SetTitle("SAVED RULES");
	savedRulesCard->SetShowFooter(false);

	savedRulesList = new RuleListControl(savedRulesCard, this);
	savedRulesCard->GetContentSizer()->Add(savedRulesList, wxSizerFlags(1).Expand().Border(wxALL, padding));

	// -- Bottom Card: Actions --
	CardPanel* actionsCard = new CardPanel(splitter, wxID_ANY);
	actionsCard->SetTitle("ACTIONS");
	actionsCard->SetShowFooter(false);

	wxBoxSizer* actionsSizer = new wxBoxSizer(wxVERTICAL);

	// Grid for management buttons (Add, Edit, Del)
	wxBoxSizer* manageBtnsSizer = new wxBoxSizer(wxHORIZONTAL);

	m_addRuleBtn = new wxButton(actionsCard, wxID_ANY, "ADD", wxDefaultPosition, FromDIP(wxSize(-1, 28)));
	m_addRuleBtn->SetBackgroundColour(wxColour(40, 180, 40)); // Green
	m_addRuleBtn->SetForegroundColour(*wxWHITE);
	m_addRuleBtn->SetFont(Theme::GetFont(9, true));

	m_editRuleBtn = new wxButton(actionsCard, wxID_ANY, "EDIT", wxDefaultPosition, FromDIP(wxSize(-1, 28)));
	m_editRuleBtn->SetBackgroundColour(wxColour(80, 80, 80)); // Neutral Dark Gray
	m_editRuleBtn->SetForegroundColour(*wxWHITE);
	m_editRuleBtn->SetFont(Theme::GetFont(9, true));

	m_deleteRuleBtn = new wxButton(actionsCard, wxID_ANY, "DEL", wxDefaultPosition, FromDIP(wxSize(-1, 28)));
	m_deleteRuleBtn->SetBackgroundColour(wxColour(180, 40, 40)); // Red
	m_deleteRuleBtn->SetForegroundColour(*wxWHITE);
	m_deleteRuleBtn->SetFont(Theme::GetFont(9, true));

	manageBtnsSizer->Add(m_addRuleBtn, wxSizerFlags(1).Border(wxRIGHT, 2));
	manageBtnsSizer->Add(m_editRuleBtn, wxSizerFlags(1).Border(wxLEFT | wxRIGHT, 2));
	manageBtnsSizer->Add(m_deleteRuleBtn, wxSizerFlags(1).Border(wxLEFT, 2));

	actionsSizer->Add(manageBtnsSizer, wxSizerFlags(0).Expand().Border(wxALL, padding));

	m_addVisibleBtn = new wxButton(actionsCard, wxID_ANY, "ADD VISIBLE FROM VIEWPORT", wxDefaultPosition, FromDIP(wxSize(-1, 28)));
	m_addVisibleBtn->SetBackgroundColour(wxColour(45, 120, 180)); // Sky Blue
	m_addVisibleBtn->SetForegroundColour(*wxWHITE);
	m_addVisibleBtn->SetFont(Theme::GetFont(9, true));
	actionsSizer->Add(m_addVisibleBtn, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT | wxBOTTOM, padding));

	// Scope Selection
	wxBoxSizer* scopeSizer = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText* scopeLabel = new wxStaticText(actionsCard, wxID_ANY, "SCOPE:");
	scopeLabel->SetForegroundColour(wxColour(180, 180, 180));
	scopeLabel->SetFont(Theme::GetFont(9, true));

	wxArrayString scopes;
	scopes.Add("SELECTION");
	scopes.Add("VIEWPORT");
	scopes.Add("ALL MAP");
	m_scopeChoice = new wxChoice(actionsCard, wxID_ANY, wxDefaultPosition, FromDIP(wxSize(-1, 24)), scopes);
	m_scopeChoice->SetSelection(0);
	if (editor && editor->selection.empty()) {
		m_scopeChoice->SetSelection(1); // Default to Viewport if no selection
	}

	scopeSizer->Add(scopeLabel, wxSizerFlags(0).Center().Border(wxRIGHT, 5));
	scopeSizer->Add(m_scopeChoice, wxSizerFlags(1).Center());

	actionsSizer->Add(scopeSizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT | wxBOTTOM, padding));

	actionsSizer->AddStretchSpacer(1);

	// Execute Button
	m_executeBtn = new wxButton(actionsCard, wxID_ANY, "EXECUTE REPLACE", wxDefaultPosition, FromDIP(wxSize(-1, 32)));
	m_executeBtn->SetDefault();
	m_executeBtn->SetBackgroundColour(Theme::Get(Theme::Role::Accent)); // Blue
	m_executeBtn->SetForegroundColour(*wxWHITE);
	m_executeBtn->SetFont(Theme::GetFont(10, true));
	actionsSizer->Add(m_executeBtn, wxSizerFlags(0).Expand().Border(wxALL, padding));

	// Close Button
	wxButton* closeBtn = new wxButton(actionsCard, wxID_ANY, "CLOSE", wxDefaultPosition, FromDIP(wxSize(-1, 32)));
	closeBtn->SetBackgroundColour(wxColour(120, 120, 120)); // Gray
	closeBtn->SetForegroundColour(*wxWHITE);
	closeBtn->SetFont(Theme::GetFont(10, true));
	actionsSizer->Add(closeBtn, wxSizerFlags(0).Expand().Border(wxALL, padding));

	actionsCard->GetContentSizer()->Add(actionsSizer, wxSizerFlags(1).Expand());

	// Setup Splitter
	splitter->SetMinimumPaneSize(FromDIP(100));
	splitter->SplitHorizontally(savedRulesCard, actionsCard);
	splitter->SetSashGravity(0.65); // Give 65% to the list

	mainRowSizer->Add(splitter, wxSizerFlags(2).Expand().Border(wxALL, padding / 2)); // Flex 2

	// ---------------------------------------------------------
	// BINDINGS
	// ---------------------------------------------------------
	m_executeBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnExecute, this);
	m_addVisibleBtn->Bind(wxEVT_BUTTON, &ReplaceToolWindow::OnAddVisibleTiles, this);
	closeBtn->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Close(); });

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
		wxMessageBox("No rules defined. Add some rules first.", "Replace", wxOK | wxICON_INFORMATION);
		return;
	}

	int scopeIdx = m_scopeChoice->GetSelection();
	ReplaceScope scope = ReplaceScope::Selection;
	if (scopeIdx == 1) {
		scope = ReplaceScope::Viewport;
	} else if (scopeIdx == 2) {
		scope = ReplaceScope::AllMap;
	}

	if (scope == ReplaceScope::Selection && editor->selection.empty()) {
		wxMessageBox("No selection active. Please select an area first or change scope.", "Replace", wxOK | wxICON_WARNING);
		return;
	}

	int confirm = wxMessageBox(
		"Are you sure you want to execute replacements on the map?",
		"Confirm Bulk Replacement",
		wxYES_NO | wxNO_DEFAULT | wxICON_WARNING
	);

	if (confirm != wxYES) {
		return;
	}

	if (scope == ReplaceScope::Viewport) {
		MapTab* activeTab = g_gui.GetCurrentMapTab();
		if (activeTab && activeTab->GetCanvas()) {
			MapCanvas* canvas = activeTab->GetCanvas();
			wxSize canvasSize = canvas->GetClientSize();

			int startX, startY, endX, endY;
			canvas->ScreenToMap(0, 0, &startX, &startY);
			canvas->ScreenToMap(canvasSize.x, canvasSize.y, &endX, &endY);

			// Ensure bounds are consistent
			if (startX > endX) {
				std::swap(startX, endX);
			}
			if (startY > endY) {
				std::swap(startY, endY);
			}

			int z = canvas->GetFloor();

			std::vector<Position> pv;
			for (int x = startX; x <= endX; ++x) {
				for (int y = startY; y <= endY; ++y) {
					pv.push_back(Position(x, y, z));
				}
			}
			engine.ExecuteReplacement(editor, rules, scope, &pv);
		}
	} else {
		engine.ExecuteReplacement(editor, rules, scope);
	}
}

void ReplaceToolWindow::OnAddVisibleTiles(wxCommandEvent&) {
	MapTab* activeTab = g_gui.GetCurrentMapTab();
	if (!activeTab || !activeTab->GetCanvas()) {
		return;
	}

	MapCanvas* canvas = activeTab->GetCanvas();
	wxSize canvasSize = canvas->GetClientSize();

	int startX, startY, endX, endY;
	canvas->ScreenToMap(0, 0, &startX, &startY);
	canvas->ScreenToMap(canvasSize.x, canvasSize.y, &endX, &endY);

	// Ensure bounds are consistent
	if (startX > endX) {
		std::swap(startX, endX);
	}
	if (startY > endY) {
		std::swap(startY, endY);
	}

	int z = canvas->GetFloor();

	std::set<uint16_t> uniqueIds;
	for (int x = startX; x <= endX; ++x) {
		for (int y = startY; y <= endY; ++y) {
			Tile* tile = editor->map.getTile(x, y, z);
			if (tile) {
				if (tile->ground) {
					uniqueIds.insert(tile->ground->getID());
				}
				for (Item* item : tile->items) {
					uniqueIds.insert(item->getID());
				}
			}
		}
	}

	if (uniqueIds.empty()) {
		return;
	}

	std::vector<ReplacementRule> currentRules = ruleBuilder->GetRules();
	std::set<uint16_t> existingIds;
	for (const auto& r : currentRules) {
		existingIds.insert(r.fromId);
	}

	bool added = false;
	for (uint16_t id : uniqueIds) {
		if (id != 0 && existingIds.find(id) == existingIds.end()) {
			ReplacementRule nr;
			nr.fromId = id;
			currentRules.push_back(nr);
			added = true;
		}
	}

	if (added) {
		ruleBuilder->SetRules(currentRules);
	}
}

void ReplaceToolWindow::OnRuleItemSelected(uint16_t itemId) {
	OnLibraryItemSelected(itemId);
}

void ReplaceToolWindow::OnSaveRule() {
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
		return;
	}
	OnLibraryItemSelected(itemId);
}
