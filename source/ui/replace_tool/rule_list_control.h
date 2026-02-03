#ifndef RME_RULE_LIST_CONTROL_H_
#define RME_RULE_LIST_CONTROL_H_

#include "app/main.h"
#include "ui/replace_tool/rule_manager.h"
#include <wx/control.h>
#include <vector>

class RuleListControl : public wxControl {
public:
	class Listener {
	public:
		virtual ~Listener() { }
		virtual void OnRuleSelected(const RuleSet& ruleSet) = 0;
	};

	RuleListControl(wxWindow* parent, Listener* listener);

	void SetRuleSets(const std::vector<std::string>& ruleSetNames);
	wxSize DoGetBestClientSize() const override;

private:
	void OnPaint(wxPaintEvent& event);
	void OnMouse(wxMouseEvent& event);
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
