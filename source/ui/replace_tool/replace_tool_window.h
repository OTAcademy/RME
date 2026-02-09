#ifndef RME_REPLACE_TOOL_WINDOW_H_
#define RME_REPLACE_TOOL_WINDOW_H_

#include "app/main.h"
#include "ui/replace_tool/library_panel.h"
#include "ui/replace_tool/rule_manager.h"
#include "ui/replace_tool/replacement_engine.h"
#include "ui/replace_tool/rule_list_control.h"
#include "ui/replace_tool/rule_builder_panel.h"
#include <wx/srchctrl.h>
#include <wx/notebook.h>
#include <wx/splitter.h>
#include <wx/dialog.h>
#include <map>

class Editor;

class ReplaceToolWindow : public wxDialog, public LibraryPanel::Listener, public RuleListControl::Listener, public RuleBuilderPanel::Listener, public ItemGridPanel::Listener {
public:
	ReplaceToolWindow(wxWindow* parent, Editor* editor);
	virtual ~ReplaceToolWindow();

	void InitializeWithIDs(const std::vector<uint16_t>& ids);

	// LibraryPanel::Listener
	void OnLibraryItemSelected(uint16_t itemId) override;

	// RuleListControl::Listener
	virtual void OnRuleSelected(const RuleSet& rs) override;
	virtual void OnRuleDeleted(const std::string& name) override;
	virtual void OnRuleRenamed(const std::string& oldName, const std::string& newName) override;
	// RuleBuilderPanel::Listener
	virtual void OnRuleChanged() override;
	virtual void OnClearRules() override;
	virtual void OnSaveRule() override;
	virtual void OnRuleItemSelected(uint16_t itemId) override;

	// ItemGridPanel::Listener
	virtual void OnItemSelected(ItemGridPanel* source, uint16_t itemId) override;

	void OnClose(wxCloseEvent& event);

private:
	void InitLayout();
	void UpdateSavedRulesList();

	// Event Handlers
	void OnExecute(wxCommandEvent& event);
	void OnAddVisibleTiles(wxCommandEvent& event);

	Editor* editor;

	LibraryPanel* libraryPanel;
	ItemGridPanel* similarItemsGrid;
	RuleBuilderPanel* ruleBuilder;
	RuleListControl* savedRulesList;
	// wxButton* m_saveBtn; // Removed
	wxButton* m_executeBtn;
	wxButton* m_addRuleBtn;
	wxButton* m_editRuleBtn;
	wxButton* m_deleteRuleBtn;
	wxButton* m_addVisibleBtn;
	wxChoice* m_scopeChoice;

	ReplacementEngine engine;

	std::string m_activeRuleSetName;
};

#endif
