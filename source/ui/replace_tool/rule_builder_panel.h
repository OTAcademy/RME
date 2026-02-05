#ifndef RME_RULE_BUILDER_PANEL_H_
#define RME_RULE_BUILDER_PANEL_H_

#include "app/main.h"
#include "ui/replace_tool/rule_manager.h"
#include "util/nanovg_canvas.h"
#include "ui/replace_tool/rule_manager.h"
#include <wx/dnd.h>

class RuleBuilderPanel : public NanoVGCanvas {
public:
	class Listener {
	public:
		virtual ~Listener() { }
		virtual void OnRuleChanged() = 0;
	};

	RuleBuilderPanel(wxWindow* parent, Listener* listener);
	virtual ~RuleBuilderPanel();

	void SetRules(const std::vector<ReplacementRule>& rules);
	std::vector<ReplacementRule> GetRules() const;
	void Clear();

	// wxControl overrides
	wxSize DoGetBestClientSize() const override;

protected:
	void OnNanoVGPaint(NVGcontext* vg, int width, int height) override;

private:
	void OnMouse(wxMouseEvent& event);
	void OnSize(wxSizeEvent& event);

	// Hit testing
	struct HitResult {
		enum Type {
			None,
			Source, // The source item icon
			Target, // A specific target item icon
			AddTarget, // The [+] ghost slot at the end of targets
			NewRule, // The large "Drop New Rule" area
			DeleteRule, // The 'X' on the rule card
			DeleteTarget // The 'X' overlay on a specific target
		};
		Type type = None;
		int ruleIndex = -1;
		int targetIndex = -1;
	};
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

	std::vector<ReplacementRule> m_rules;
	Listener* m_listener;

	// Visual Layout
	int m_rowHeight;
	int m_sourceColWidth;
	void LayoutRules();

	// Drag feedback
	HitResult m_dragHover;

	wxTimer m_pulseTimer;
	void OnPulseTimer(wxTimerEvent& event);

	friend class ItemDropTarget;
};

#endif
