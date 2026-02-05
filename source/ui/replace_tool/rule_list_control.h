#ifndef RME_RULE_LIST_CONTROL_H_
#define RME_RULE_LIST_CONTROL_H_

#include "ui/replace_tool/rule_manager.h"
#include <wx/control.h>
#include <vector>
#include <string>

class RuleListControl : public wxControl {
public:
	class Listener {
	public:
		virtual ~Listener() { }
		virtual void OnRuleSelected(const RuleSet& ruleSet) = 0;
		virtual void OnRuleDeleted(const std::string& name) = 0;
		virtual void OnRuleRenamed(const std::string& oldName, const std::string& newName) = 0;
	};

	RuleListControl(wxWindow* parent, Listener* listener);

	void SetRuleSets(const std::vector<std::string>& ruleSetNames);
	wxSize DoGetBestClientSize() const override;

	int GetSelectedIndex() const {
		return m_selectedIndex;
	}
	const std::vector<std::string>& GetRuleSetNames() const {
		return m_ruleSetNames;
	}

private:
	void OnPaint(wxPaintEvent& event);
	void OnMouse(wxMouseEvent& event);
	void OnContextMenu(wxContextMenuEvent& event);
	void OnScroll(wxScrollWinEvent& event);
	void OnSize(wxSizeEvent& event);

	void RefreshVirtualSize();
	wxRect GetItemRect(int index) const;

	std::vector<std::string> m_ruleSetNames;
	int m_selectedIndex = -1;
	int m_hoveredIndex = -1;
	int m_itemHeight;
	Listener* m_listener;
};

#endif
