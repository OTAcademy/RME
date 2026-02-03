#ifndef RME_REPLACE_TOOL_WINDOW_H_
#define RME_REPLACE_TOOL_WINDOW_H_

#include "app/main.h"
#include "ui/replace_tool/item_grid_panel.h"
#include "ui/replace_tool/rule_manager.h"
#include "ui/replace_tool/replacement_engine.h"
#include "ui/replace_tool/rule_list_control.h"
#include "ui/replace_tool/rule_builder_panel.h"
#include <wx/srchctrl.h>
#include <wx/notebook.h>
#include <map>

class Editor;

class ReplaceToolWindow : public wxDialog, public ItemGridPanel::Listener, public RuleListControl::Listener, public RuleBuilderPanel::Listener {
public:
	ReplaceToolWindow(wxWindow* parent, Editor* editor);
	virtual ~ReplaceToolWindow();

	// ItemGridPanel::Listener
	void OnItemSelected(ItemGridPanel* source, uint16_t itemId) override;

	// RuleListControl::Listener
	void OnRuleSelected(const RuleSet& ruleSet) override;

	// RuleBuilderPanel::Listener
	void OnRuleChanged() override;

private:
	void InitLayout();
	void UpdateSavedRulesList();

	// Event Handlers
	void OnSearchChange(wxCommandEvent& event);
	void OnExecute(wxCommandEvent& event);
	void OnSaveRule(wxCommandEvent& event);
	void OnClearSource(wxCommandEvent& event);

	Editor* editor;

	ItemGridPanel* allItemsGrid;
	ItemGridPanel* similarItemsGrid;
	RuleBuilderPanel* ruleBuilder;
	RuleListControl* savedRulesList;
	wxSearchCtrl* searchCtrl;
	wxButton* m_saveBtn;
	wxButton* m_executeBtn;

	ReplacementEngine engine;

	// Helper
	void PopulateBrushGrid();
	void PopulateRelatedItems(uint16_t brushLookId);
	void OnBrushSearchChange(wxCommandEvent& event);

	// New components
	wxNotebook* libraryTabs;
	wxSearchCtrl* brushSearchCtrl;
	ItemGridPanel* brushListGrid;
	ItemGridPanel* brushRelatedGrid;

	std::map<uint16_t, uint16_t> cidToSidCache;
	uint16_t GetSidFromCid(uint16_t cid);

	// Map lookup from LookID -> Brush* for easier access when selecting a brush
	std::map<uint16_t, class Brush*> brushLookup;
};

#endif
