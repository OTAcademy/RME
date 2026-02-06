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
#include <map>

class Editor;

class ReplaceToolWindow : public wxDialog, public LibraryPanel::Listener, public RuleListControl::Listener, public RuleBuilderPanel::Listener, public ItemGridPanel::Listener {
public:
	ReplaceToolWindow(wxWindow* parent, Editor* editor);
	virtual ~ReplaceToolWindow();

	// LibraryPanel::Listener
	void OnLibraryItemSelected(uint16_t itemId) override;

	// RuleListControl::Listener
	virtual void OnRuleSelected(const RuleSet& rs) override;
	virtual void OnRuleDeleted(const std::string& name) override;
	virtual void OnRuleRenamed(const std::string& oldName, const std::string& newName) override;
	// RuleBuilderPanel::Listener
	virtual void OnRuleChanged() override;
	virtual void OnClearRules() override;

	// ItemGridPanel::Listener
	virtual void OnItemSelected(ItemGridPanel* source, uint16_t itemId) override;

private:
	void InitLayout();
	void UpdateSavedRulesList();

	// Event Handlers
	void OnExecute(wxCommandEvent& event);
	void OnSaveRule(wxCommandEvent& event);

	Editor* editor;

	LibraryPanel* libraryPanel;
	ItemGridPanel* similarItemsGrid;
	RuleBuilderPanel* ruleBuilder;
	RuleListControl* savedRulesList;
	wxButton* m_saveBtn;
	wxButton* m_executeBtn;
	wxButton* m_addRuleBtn;
	wxButton* m_editRuleBtn;
	wxButton* m_deleteRuleBtn;

	ReplacementEngine engine;

	std::string m_activeRuleSetName;
};

#endif
