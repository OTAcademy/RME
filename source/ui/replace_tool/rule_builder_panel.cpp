#include "ui/replace_tool/rule_builder_panel.h"
#include "ui/theme.h"
#include "game/items.h"
#include "ui/gui.h"
#include "app/managers/version_manager.h"
#include "util/nvg_utils.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/msgdlg.h>
#include <nanovg.h>
#include <format>
#include "rendering/core/text_renderer.h"

// ----------------------------------------------------------------------------
// ItemDropTarget
// ----------------------------------------------------------------------------

RuleBuilderPanel::ItemDropTarget::ItemDropTarget(RuleBuilderPanel* panel) : m_panel(panel) { }

bool RuleBuilderPanel::ItemDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data) {
	if (!data.StartsWith("RME_ITEM:")) {
		return false;
	}

	unsigned long idVal;
	if (!data.AfterFirst(':').ToULong(&idVal)) {
		return false;
	}
	uint16_t itemId = (uint16_t)idVal;

	RuleBuilderPanel::HitResult hit = m_panel->HitTest(x, y);

	if (hit.type == RuleBuilderPanel::HitResult::Source) {
		if (hit.ruleIndex >= 0 && hit.ruleIndex < (int)m_panel->m_rules.size()) {
			m_panel->m_rules[hit.ruleIndex].fromId = itemId;
			m_panel->m_listener->OnRuleChanged();
			m_panel->Refresh();
			return true;
		}
	} else if (hit.type == RuleBuilderPanel::HitResult::Target) {
		if (hit.ruleIndex >= 0 && hit.ruleIndex < (int)m_panel->m_rules.size()) {
			m_panel->m_rules[hit.ruleIndex].targets.push_back({ itemId, 0 });
			m_panel->DistributeProbabilities(hit.ruleIndex);
			m_panel->m_listener->OnRuleChanged();
			m_panel->Refresh();
			return true;
		}
	} else if (hit.type == RuleBuilderPanel::HitResult::NewRule) {
		// Create a new rule with this item as Source
		ReplacementRule newRule;
		newRule.fromId = itemId;
		m_panel->m_rules.push_back(newRule);
		m_panel->m_listener->OnRuleChanged();
		m_panel->UpdateScrollbar((int)(m_panel->m_rules.size() + 1) * m_panel->m_rowHeight + m_panel->FromDIP(24));
		m_panel->Refresh();
		return true;
	}

	return false;
}

wxDragResult RuleBuilderPanel::ItemDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def) {
	RuleBuilderPanel::HitResult hit = m_panel->HitTest(x, y);
	m_panel->m_dragHover = hit;
	m_panel->Refresh();
	return def;
}

void RuleBuilderPanel::ItemDropTarget::OnLeave() {
	m_panel->m_dragHover = { RuleBuilderPanel::HitResult::None, -1, -1 };
	m_panel->Refresh();
}

// ----------------------------------------------------------------------------
// RuleBuilderPanel
// ----------------------------------------------------------------------------

RuleBuilderPanel::RuleBuilderPanel(wxWindow* parent, Listener* listener) :
	NanoVGCanvas(parent, wxID_ANY),
	m_listener(listener) {

	m_rowHeight = FromDIP(80);
	m_sourceColWidth = FromDIP(120);

	SetDropTarget(new ItemDropTarget(this));

	Bind(wxEVT_SIZE, &RuleBuilderPanel::OnSize, this);
	Bind(wxEVT_LEFT_DOWN, &RuleBuilderPanel::OnMouse, this);
	Bind(wxEVT_RIGHT_DOWN, [this](wxMouseEvent& evt) {
		HitResult hit = HitTest(evt.GetX(), evt.GetY());
		if (hit.type == HitResult::Source && hit.ruleIndex != -1) {
			// Remove entire rule
			if (hit.ruleIndex < m_rules.size()) {
				m_rules.erase(m_rules.begin() + hit.ruleIndex);
				if (m_listener) {
					m_listener->OnRuleChanged();
				}
				UpdateScrollbar((int)(m_rules.size() + 1) * m_rowHeight + FromDIP(24));
				Refresh();
			}
		} else if (hit.type == HitResult::Target && hit.ruleIndex != -1 && hit.targetIndex != -1) {
			// Remove specific target
			auto& targets = m_rules[hit.ruleIndex].targets;
			if (hit.targetIndex < targets.size()) {
				targets.erase(targets.begin() + hit.targetIndex);
				DistributeProbabilities(hit.ruleIndex);
				if (m_listener) {
					m_listener->OnRuleChanged();
				}
				Refresh();
			}
		}
	});

	m_dragHover = { HitResult::None, -1, -1 };
}

RuleBuilderPanel::~RuleBuilderPanel() { }

void RuleBuilderPanel::SetRules(const std::vector<ReplacementRule>& rules) {
	m_rules = rules;
	UpdateScrollbar((int)(m_rules.size() + 1) * m_rowHeight + FromDIP(24));
	Refresh();
}

std::vector<ReplacementRule> RuleBuilderPanel::GetRules() const {
	return m_rules;
}

void RuleBuilderPanel::Clear() {
	m_rules.clear();
	UpdateScrollbar(FromDIP(24) + m_rowHeight);
	Refresh();
}

void RuleBuilderPanel::DistributeProbabilities(int ruleIndex) {
	if (ruleIndex < 0 || ruleIndex >= m_rules.size()) {
		return;
	}
	auto& targets = m_rules[ruleIndex].targets;

	if (targets.empty()) {
		return;
	}
	int count = targets.size();
	int prob = 100 / count;
	int remainder = 100 % count;

	for (size_t i = 0; i < targets.size(); ++i) {
		targets[i].probability = prob;
		if (i < remainder) {
			targets[i].probability++;
		}
	}
}

wxSize RuleBuilderPanel::DoGetBestClientSize() const {
	return wxSize(FromDIP(400), FromDIP(300));
}

void RuleBuilderPanel::OnSize(wxSizeEvent& event) {
	UpdateScrollbar((int)(m_rules.size() + 1) * m_rowHeight + FromDIP(24));
	Refresh();
	event.Skip();
}

RuleBuilderPanel::HitResult RuleBuilderPanel::HitTest(int x, int y) const {
	int headerHeight = FromDIP(24);
	int scrollPos = GetScrollPosition();
	int contentY = y + scrollPos;

	if (y < headerHeight) {
		return { HitResult::None, -1, -1 };
	}

	int relY = contentY - headerHeight;
	int rowIndex = relY / m_rowHeight;

	if (rowIndex < (int)m_rules.size()) {
		if (x < m_sourceColWidth) {
			return { HitResult::Source, rowIndex, -1 };
		} else {
			// Check specific target
			int targetX = x - m_sourceColWidth;
			int itemWithPad = FromDIP(68); // 64 item + 4 pad
			int targetIdx = targetX / itemWithPad;

			if (targetIdx < m_rules[rowIndex].targets.size()) {
				return { HitResult::Target, rowIndex, targetIdx };
			}
			return { HitResult::Target, rowIndex, -1 };
		}
	}

	return { HitResult::NewRule, -1, -1 };
}

void RuleBuilderPanel::OnMouse(wxMouseEvent& event) {
	event.Skip();
}

void RuleBuilderPanel::OnNanoVGPaint(NVGcontext* vg, int width, int height) {
	int headerHeight = FromDIP(24);

	int y = headerHeight;
	int scrollPos = GetScrollPosition();

	wxColour textCol = Theme::Get(Theme::Role::Text);

	// Create common colors
	NVGcolor colBg = nvgRGBA(40, 40, 40, 255);
	NVGcolor colText = nvgRGBA(255, 255, 255, 255);
	NVGcolor colSubText = nvgRGBA(180, 180, 180, 255);
	NVGcolor colBorder = nvgRGBA(60, 60, 60, 255);
	NVGcolor colAccent = nvgRGBA(Theme::Get(Theme::Role::Accent).Red(), Theme::Get(Theme::Role::Accent).Green(), Theme::Get(Theme::Role::Accent).Blue(), 255);

	// Header
	nvgBeginPath(vg);
	nvgMoveTo(vg, 0, headerHeight);
	nvgLineTo(vg, width, headerHeight);
	nvgStrokeColor(vg, colBorder);
	nvgStrokeWidth(vg, 1.0f);
	nvgStroke(vg);

	nvgFontSize(vg, 12.0f);
	nvgFontFace(vg, "sans");
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgFillColor(vg, colSubText);
	nvgText(vg, 10, headerHeight / 2.0f, "Original", nullptr);
	nvgText(vg, m_sourceColWidth + 10, headerHeight / 2.0f, "Replacements", nullptr);

	// Draw Rules
	for (size_t i = 0; i < m_rules.size(); ++i) {
		const ReplacementRule& rule = m_rules[i];

		// Alternating row background
		if (i % 2 == 1) {
			nvgBeginPath(vg);
			nvgRect(vg, 0, y, width, m_rowHeight);
			nvgFillColor(vg, colBg);
			nvgFill(vg);
		}

		// Source Item
		if (rule.fromId != 0) {
			int tex = GetCachedImage(rule.fromId);
			if (tex == 0) {
				tex = NvgUtils::CreateItemTexture(vg, rule.fromId);
				if (tex > 0) {
					AddCachedImage(rule.fromId, tex);
				}
			}

			if (tex > 0) {
				int tw, th;
				nvgImageSize(vg, tex, &tw, &th);
				float scale = 64.0f / (float)std::max(tw, th);
				float dw = (float)tw * scale;
				float dh = (float)th * scale;

				float imgX = 10;
				float imgY = y + (m_rowHeight - 64) / 2.0f;

				NVGpaint imgPaint = nvgImagePattern(vg, imgX + (64 - dw) / 2.0f, imgY + (64 - dh) / 2, dw, dh, 0.0f, tex, 1.0f);
				nvgBeginPath(vg);
				nvgRect(vg, imgX + (64 - dw) / 2.0f, imgY + (64 - dh) / 2, dw, dh);
				nvgFillPaint(vg, imgPaint);
				nvgFill(vg);
			}

			// ID Text
			nvgFillColor(vg, colText);
			nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
			nvgText(vg, 10 + 64 + 10, y + m_rowHeight / 2.0f, std::format("{}", rule.fromId).c_str(), nullptr);
		}

		// Highlight Source Drop
		if (m_dragHover.type == HitResult::Source && m_dragHover.ruleIndex == (int)i) {
			nvgBeginPath(vg);
			nvgRect(vg, 1, y + 1, m_sourceColWidth - 2, m_rowHeight - 2);
			nvgStrokeColor(vg, colAccent);
			nvgStrokeWidth(vg, 2.0f);
			nvgStroke(vg);
		}

		// Targets
		int x = m_sourceColWidth;
		int itemWithPad = FromDIP(68);

		for (size_t t = 0; t < rule.targets.size(); ++t) {
			const auto& target = rule.targets[t];

			int tex = GetCachedImage(target.id);
			if (tex == 0) {
				tex = NvgUtils::CreateItemTexture(vg, target.id);
				if (tex > 0) {
					AddCachedImage(target.id, tex);
				}
			}

			if (tex > 0) {
				int tw, th;
				nvgImageSize(vg, tex, &tw, &th);
				float scale = 64.0f / (float)std::max(tw, th);
				float dw = (float)tw * scale;
				float dh = (float)th * scale;

				float imgX = x; // Starts after source col
				float imgY = y + (m_rowHeight - 64) / 2.0f;

				NVGpaint imgPaint = nvgImagePattern(vg, imgX + (64 - dw) / 2.0f, imgY + (64 - dh) / 2, dw, dh, 0.0f, tex, 1.0f);
				nvgBeginPath(vg);
				nvgRect(vg, imgX + (64 - dw) / 2.0f, imgY + (64 - dh) / 2, dw, dh);
				nvgFillPaint(vg, imgPaint);
				nvgFill(vg);
			}

			// Probability overlay
			if (target.probability != 100) {
				std::string prob = std::to_string(target.probability);
				nvgFontSize(vg, 10.0f);
				nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);

				float txtX = x + 64 - 2;
				float txtY = y + (m_rowHeight - 64) / 2.0f + 64 - 2;

				nvgFillColor(vg, nvgRGBA(0, 0, 0, 180));

				// Bg check not trivially easy for text bounds without more calc, skip bg for now or assume size
				// nvgTextBounds...

				nvgFillColor(vg, colText);
				nvgText(vg, txtX, txtY, prob.c_str(), nullptr);
			}

			x += itemWithPad;
		}

		// Highlight Target Drop
		if (m_dragHover.type == HitResult::Target && m_dragHover.ruleIndex == (int)i) {
			nvgBeginPath(vg);
			nvgRect(vg, x, y + (m_rowHeight - 64) / 2, 64, 64);
			nvgStrokeColor(vg, colAccent);
			nvgStrokeWidth(vg, 2.0f);
			nvgStroke(vg);
		}

		// Divider
		nvgBeginPath(vg);
		nvgMoveTo(vg, 0, y + m_rowHeight);
		nvgLineTo(vg, width, y + m_rowHeight);
		nvgStrokeColor(vg, colBorder);
		nvgStrokeWidth(vg, 1.0f);
		nvgStroke(vg);

		y += m_rowHeight;
	}

	// New Rule Drop
	if (m_dragHover.type == HitResult::NewRule) {
		nvgBeginPath(vg);
		nvgRect(vg, 10, y + 10, width - 20, m_rowHeight);
		nvgStrokeColor(vg, colAccent);
		// Dash? nv features not simple dash.
		nvgStrokeWidth(vg, 2.0f);
		nvgStroke(vg);

		nvgFillColor(vg, colAccent);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, width / 2.0f, y + m_rowHeight / 2.0f + 10, "Drop to Add New Rule", nullptr);
	} else if (m_rules.empty()) {
		nvgFillColor(vg, colSubText);
		nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
		nvgText(vg, width / 2.0f, y + 30.0f, "Drag items here to create rules", nullptr);
	}
}
