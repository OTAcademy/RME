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
		enum Type { None,
					Source,
					Target,
					NewRule // Drop area for creating a new rule
		};
		Type type = None; // Default init
		int ruleIndex = -1; // Index into m_rules
		int targetIndex = -1; // Index into m_rules[ruleIndex].targets (if Target)
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

	// Drag feedback
	HitResult m_dragHover;

	friend class ItemDropTarget;
};

#endif
