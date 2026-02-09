#ifndef RME_RULE_BUILDER_PANEL_H_
#define RME_RULE_BUILDER_PANEL_H_

#include "ui/replace_tool/rule_manager.h"
#include "util/nanovg_canvas.h"
#include <wx/wx.h>
#include <wx/control.h>
#include <wx/dnd.h>
#include <vector>
#include <string>
#include <cstdint>

class RuleBuilderPanel : public NanoVGCanvas {
public:
	class Listener {
	public:
		virtual ~Listener() = default;
		virtual void OnRuleChanged() = 0;
		virtual void OnClearRules() = 0;
		virtual void OnSaveRule() = 0;
		virtual void OnRuleItemSelected(uint16_t itemId) = 0;
	};

	// Hit testing results (Public for renderer access)
	struct HitResult {
		enum Type {
			None,
			Source, // The source item icon
			Target, // A specific target item icon
			AddTarget, // The [+] ghost slot at the end of targets
			NewRule, // The large "Drop New Rule" area
			ClearRules,
			SaveRule,
			DeleteRule, // The 'X' on the rule card
			DeleteTarget, // The 'X' overlay on a specific target
		};
		Type type = None;
		int ruleIndex = -1;
		int targetIndex = -1;
	};

	RuleBuilderPanel(wxWindow* parent, Listener* listener);
	virtual ~RuleBuilderPanel();
	HitResult HitTest(int x, int y) const;
	void DistributeProbabilities(int ruleIndex);

	// Drop Target
	class ItemDropTarget : public wxTextDropTarget {
	public:
		ItemDropTarget(RuleBuilderPanel* panel);
		bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
		wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;
		void OnLeave() override;

	private:
		RuleBuilderPanel* m_panel;
	};

	void Clear();
	void SetRules(const std::vector<ReplacementRule>& rules);
	std::vector<ReplacementRule> GetRules() const;

	// Visual Layout
	int GetRuleHeight(int index, int width) const;
	int GetRuleY(int index, int width) const;
	void LayoutRules();

protected:
	virtual void OnNanoVGPaint(NVGcontext* vg, int width, int height) override;
	virtual wxSize DoGetBestClientSize() const override;

	void OnSize(wxSizeEvent& event);
	void OnMouse(wxMouseEvent& event);

private:
	std::vector<ReplacementRule> m_rules;
	wxSize m_lastSize;
	Listener* m_listener;

	// Drag feedback
	HitResult m_dragHover;
	bool m_isExternalDrag = false;

	// Layout Cache
	mutable std::vector<int> m_ruleYCache;
	mutable int m_totalHeight = 0;
};

#endif
