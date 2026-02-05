#include "ui/replace_tool/card_panel.h"
#include "ui/theme.h"
#include <wx/dcclient.h>
#include <wx/graphics.h>
#include <wx/settings.h>

CardPanel::CardPanel(wxWindow* parent, wxWindowID id) : wxPanel(parent, id) {
	SetBackgroundStyle(wxBG_STYLE_PAINT);
	Bind(wxEVT_PAINT, &CardPanel::OnPaint, this);
	Bind(wxEVT_SIZE, &CardPanel::OnSize, this);

	m_mainSizer = new wxBoxSizer(wxVERTICAL);
	m_mainSizer->AddSpacer(HEADER_HEIGHT);

	m_contentSizer = new wxBoxSizer(wxVERTICAL);
	m_mainSizer->Add(m_contentSizer, 1, wxEXPAND);

	m_footerSizer = new wxBoxSizer(wxVERTICAL);
	m_mainSizer->Add(m_footerSizer, 0, wxEXPAND);

	SetSizer(m_mainSizer);
}

void CardPanel::SetShowFooter(bool show) {
	m_showFooter = show;
	if (m_showFooter) {
		m_mainSizer->SetItemMinSize(m_footerSizer, wxSize(-1, FOOTER_HEIGHT));
	} else {
		m_mainSizer->SetItemMinSize(m_footerSizer, wxSize(-1, 0));
	}
	Layout();
	Refresh();
}

// SetIsActive removed for now or just ignored

void CardPanel::SetTitle(const wxString& title) {
	m_title = title;
	Refresh();
}

void CardPanel::OnSize(wxSizeEvent& event) {
	Refresh();
	event.Skip();
}

void CardPanel::OnPaint(wxPaintEvent& event) {
	wxPaintDC dc(this);
	std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

	if (!gc) {
		return;
	}

	wxRect rect = GetClientRect();
	double w = rect.width;
	double h = rect.height;

	// Determine Colors
	// Outer background (simulated shadow/border)
	wxColour base = Theme::Get(Theme::Role::Surface);
	wxColour cardBgStart = wxColour(45, 45, 52); // Slightly bluish dark
	wxColour cardBgEnd = wxColour(40, 40, 48);
	wxColour borderColor = wxColour(60, 60, 70);
	wxColour headerBg = wxColour(35, 35, 40);
	wxColour titleColor = wxColour(220, 220, 230);

	// Clear
	gc->SetBrush(wxBrush(base));
	gc->DrawRectangle(0, 0, w, h);

	// Padding for "shadow"
	double margin = 2.0;
	double r = 6.0; // Radius

	// Draw Shadow (simulated with offset rect)
	gc->SetBrush(wxBrush(wxColour(0, 0, 0, 40)));
	gc->SetPen(*wxTRANSPARENT_PEN);
	gc->DrawRoundedRectangle(margin + 2, margin + 2, w - 2 * margin, h - 2 * margin, r);

	// Draw Card Body
	wxGraphicsPath path = gc->CreatePath();
	path.AddRoundedRectangle(margin, margin, w - 2 * margin, h - 2 * margin, r);

	// Gradient
	wxGraphicsBrush brush = gc->CreateLinearGradientBrush(margin, margin, margin, h - margin, cardBgStart, cardBgEnd);
	gc->SetBrush(brush);
	gc->SetPen(wxPen(borderColor, 1));
	gc->FillPath(path);
	gc->StrokePath(path);

	// Draw Header if Title exists
	// Draw Header if Title exists
	if (!m_title.IsEmpty()) {
		double headerH = (double)HEADER_HEIGHT;
		double x = margin;
		double y = margin;
		double cw = w - 2 * margin;
		double ch = headerH; // Height of header part

		// Create path for header (Top corners rounded, bottom square)
		wxGraphicsPath headerPath = gc->CreatePath();

		// Start at Bottom-Left of header
		headerPath.MoveToPoint(x, y + ch);

		// Left vertical up to start of round
		headerPath.AddLineToPoint(x, y + r);

		// Top-Left Corner (180 to 270 degrees) -> PI to 1.5 PI
		const double PI = 3.14159265358979323846;
		headerPath.AddArc(x + r, y + r, r, PI, 1.5 * PI, true);

		// Top Line
		headerPath.AddLineToPoint(x + cw - r, y);

		// Top-Right Corner (270 to 360 degrees) -> 1.5 PI to 2 PI
		headerPath.AddArc(x + cw - r, y + r, r, 1.5 * PI, 2.0 * PI, true);

		// Right vertical down
		headerPath.AddLineToPoint(x + cw, y + ch);

		// Bottom Line (Separator)
		headerPath.AddLineToPoint(x, y + ch);

		headerPath.CloseSubpath();

		// Fill Header
		gc->SetBrush(wxBrush(headerBg));
		gc->SetPen(*wxTRANSPARENT_PEN);
		gc->FillPath(headerPath);

		// Draw Separator Line
		gc->SetPen(wxPen(wxColour(0, 0, 0, 50), 1));
		gc->StrokeLine(x, y + ch, x + cw, y + ch);

		// Draw Text
		gc->SetFont(Theme::GetFont(9, true), titleColor);

		double tw, th, td, te;
		gc->GetTextExtent(m_title, &tw, &th, &td, &te);

		// Center text in header
		double tx = x + (cw - tw) / 2.0;
		double ty = y + (ch - th) / 2.0;

		gc->DrawText(m_title, tx, ty);
	}

	// Draw Footer if requested
	if (m_showFooter) {
		double footerH = (double)FOOTER_HEIGHT;
		double x = margin;
		double y = h - margin - footerH;
		double cw = w - 2 * margin;
		double ch = footerH;

		// Create path for footer (Bottom corners rounded, top square)
		wxGraphicsPath footerPath = gc->CreatePath();

		// Start at Top-Left of footer
		footerPath.MoveToPoint(x, y);

		// Top Line (Separator)
		footerPath.AddLineToPoint(x + cw, y);

		// Right vertical down to start of round
		footerPath.AddLineToPoint(x + cw, y + ch - r);

		// Bottom-Right Corner
		const double PI = 3.14159265358979323846;
		footerPath.AddArc(x + cw - r, y + ch - r, r, 0, 0.5 * PI, true);

		// Bottom Line
		footerPath.AddLineToPoint(x + r, y + ch);

		// Bottom-Left Corner
		footerPath.AddArc(x + r, y + ch - r, r, 0.5 * PI, PI, true);

		// Left vertical up
		footerPath.AddLineToPoint(x, y);

		footerPath.CloseSubpath();

		// Fill Footer
		gc->SetBrush(wxBrush(headerBg));
		gc->SetPen(*wxTRANSPARENT_PEN);
		gc->FillPath(footerPath);

		// Draw Separator Line
		gc->SetPen(wxPen(wxColour(0, 0, 0, 50), 1));
		gc->StrokeLine(x, y, x + cw, y);
	}
}
