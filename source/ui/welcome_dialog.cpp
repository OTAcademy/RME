#include "app/main.h"
#include "ui/welcome_dialog.h"
#include "app/settings.h"
#include "app/preferences.h"
#include "ui/controls/modern_button.h"
#include <wx/dcbuffer.h>
#include <wx/statline.h>

wxDEFINE_EVENT(WELCOME_DIALOG_ACTION, wxCommandEvent);

// Helper for recent files
class RecentFileItem : public wxPanel {
public:
	RecentFileItem(wxWindow* parent, const wxString& path, const wxString& date) :
		wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
		m_path(path), m_date(date), m_isHover(false), m_isPressed(false) {
		SetCursor(wxCursor(wxCURSOR_HAND));
		SetMinSize(wxSize(-1, FromDIP(45))); // Slightly more compact

		Bind(wxEVT_PAINT, &RecentFileItem::OnPaint, this);
		Bind(wxEVT_ENTER_WINDOW, [this](wxMouseEvent&) { m_isHover = true; Refresh(); });
		Bind(wxEVT_LEAVE_WINDOW, [this](wxMouseEvent&) { m_isHover = false; m_isPressed = false; Refresh(); });
		Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent&) { m_isPressed = true; Refresh(); });
		Bind(wxEVT_LEFT_UP, [this](wxMouseEvent& evt) {
			if (m_isPressed) {
				m_isPressed = false;
				Refresh();
				wxCommandEvent event(wxEVT_BUTTON, GetId());
				event.SetString(m_path); // Pass the path with the event
				event.SetEventObject(this);
				GetEventHandler()->ProcessEvent(event);
			}
		});
	}

	wxSize DoGetBestClientSize() const override {
		return wxSize(FromDIP(300), FromDIP(50));
	}

	void OnPaint(wxPaintEvent& evt) {
		wxAutoBufferedPaintDC dc(this);
		wxSize size = GetClientSize();

		// Background - Extremely subtle for "Quiet" UI
		wxColour bg = GetParent()->GetBackgroundColour();
		if (m_isPressed) {
			bg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT).ChangeLightness(115);
		} else if (m_isHover) {
			bg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT).ChangeLightness(105);
		}

		dc.SetBrush(wxBrush(bg));
		dc.SetPen(*wxTRANSPARENT_PEN);
		dc.DrawRectangle(size);

		// Boundaries removed for cleaner list feel (Implied via spacing)

		wxFileName fn(m_path);
		wxString filename = fn.GetFullName();
		wxString fpath = fn.GetPath();

		dc.SetFont(GetFont().Bold()); // Standard weight bold, not Larger
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		dc.DrawText(filename, FromDIP(12), FromDIP(6));

		dc.SetFont(GetFont().Smaller());
		dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
		dc.DrawText(fpath, FromDIP(12), FromDIP(24));
	}

private:
	wxString m_path;
	wxString m_date;
	bool m_isHover = false;
	bool m_isPressed = false;
};

// Main Panel Class
class WelcomeDialog::WelcomePanel : public wxPanel {
public:
	WelcomePanel(WelcomeDialog* parent, const wxBitmap& logo, const std::vector<wxString>& recentFiles) :
		wxPanel(parent) {
		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

		wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

		// --- Left Sidebar (System Shaded) ---
		wxPanel* sidebar = new wxPanel(this);
		sidebar->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		wxBoxSizer* sideSizer = new wxBoxSizer(wxVERTICAL);

		// Logo
		wxStaticBitmap* logoCtrl = new wxStaticBitmap(sidebar, wxID_ANY, logo);
		sideSizer->Add(logoCtrl, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, FromDIP(20));

		// Title
		wxStaticText* title = new wxStaticText(sidebar, wxID_ANY, "RME");
		title->SetFont(wxFontInfo(16).Bold());
		title->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		sideSizer->Add(title, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, FromDIP(15));

		// Branding Info (Version) - Discrete
		wxStaticText* version = new wxStaticText(sidebar, wxID_ANY, wxString("v") << __W_RME_VERSION__);
		version->SetFont(wxFontInfo(7));
		version->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
		sideSizer->Add(version, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, FromDIP(25));

		// Actions (Nav-style)
		auto addButton = [&](const wxString& label, int id) {
			ModernButton* btn = new ModernButton(sidebar, id, label);
			btn->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
			btn->SetMinSize(wxSize(-1, FromDIP(35))); // More compact nav height
			sideSizer->Add(btn, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, FromDIP(8));
			btn->Bind(wxEVT_BUTTON, &WelcomeDialog::OnButtonClicked, parent);
		};

		addButton("New Project", wxID_NEW);
		addButton("Open Project", wxID_OPEN);
		addButton("Preferences", wxID_PREFERENCES);

		sideSizer->AddStretchSpacer();

		sidebar->SetSizer(sideSizer);

		// --- Right Content (Recent Files) ---
		wxPanel* content = new wxPanel(this);
		content->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		wxBoxSizer* contentSizer = new wxBoxSizer(wxVERTICAL);

		wxStaticText* recentTitle = new wxStaticText(content, wxID_ANY, "Recent Projects");
		recentTitle->SetFont(wxFontInfo(12).Bold());
		recentTitle->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		contentSizer->Add(recentTitle, 0, wxLEFT | wxTOP | wxBOTTOM, FromDIP(15));

		// Scrollable list
		wxScrolledWindow* scroll = new wxScrolledWindow(content, wxID_ANY);
		scroll->SetScrollRate(0, 10);
		wxBoxSizer* listSizer = new wxBoxSizer(wxVERTICAL);

		for (const auto& file : recentFiles) {
			RecentFileItem* item = new RecentFileItem(scroll, file, "");
			listSizer->Add(item, 0, wxEXPAND | wxBOTTOM, 1);

			// Bind event to parent dialog handler
			item->Bind(wxEVT_BUTTON, &WelcomeDialog::OnRecentFileClicked, parent);
		}

		scroll->SetSizer(listSizer);
		contentSizer->Add(scroll, 1, wxEXPAND);

		// Anchored Footer for Startup Toggle
		wxPanel* footer = new wxPanel(content);
		footer->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		wxBoxSizer* footerSizer = new wxBoxSizer(wxHORIZONTAL);

		wxCheckBox* startupCheck = new wxCheckBox(footer, wxID_ANY, "Show on Startup");
		startupCheck->SetFont(wxFontInfo(8));
		startupCheck->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
		startupCheck->SetValue(g_settings.getInteger(Config::WELCOME_DIALOG) != 0);
		startupCheck->Bind(wxEVT_CHECKBOX, &WelcomeDialog::OnCheckboxClicked, parent);
		footerSizer->Add(startupCheck, 0, wxALL, FromDIP(10));

		footer->SetSizer(footerSizer);
		contentSizer->Add(footer, 0, wxEXPAND | wxTOP, 0);

		content->SetSizer(contentSizer);

		// combine
		mainSizer->Add(sidebar, 0, wxEXPAND | wxRIGHT, 0);
		mainSizer->Add(content, 1, wxEXPAND);

		SetSizer(mainSizer);
	}
};

WelcomeDialog::WelcomeDialog(const wxString& titleText, const wxString& versionText, const wxSize& size, const wxBitmap& rmeLogo, const std::vector<wxString>& recentFiles) :
	wxDialog(nullptr, wxID_ANY, titleText, wxDefaultPosition, size, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {
	Centre();
	new WelcomePanel(this, rmeLogo, recentFiles);
	SetSize(FromDIP(800), FromDIP(500));
}

void WelcomeDialog::OnButtonClicked(wxCommandEvent& event) {
	int id = event.GetId();
	if (id == wxID_PREFERENCES) {
		PreferencesWindow preferences_window(this, true);
		preferences_window.ShowModal();
		// Update UI if needed
	} else if (id == wxID_NEW) {
		wxCommandEvent newEvent(WELCOME_DIALOG_ACTION);
		newEvent.SetId(wxID_NEW);
		ProcessWindowEvent(newEvent);
	} else if (id == wxID_OPEN) {
		// Open file dialog
		wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? "(*.otbm;*.otgz)|*.otbm;*.otgz" : "(*.otbm)|*.otbm|Compressed OpenTibia Binary Map (*.otgz)|*.otgz";

		wxFileDialog file_dialog(this, "Open map file", "", "", wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
		if (file_dialog.ShowModal() == wxID_OK) {
			wxCommandEvent newEvent(WELCOME_DIALOG_ACTION);
			newEvent.SetId(wxID_OPEN);
			newEvent.SetString(file_dialog.GetPath());
			ProcessWindowEvent(newEvent);
		}
	}
}

void WelcomeDialog::OnCheckboxClicked(wxCommandEvent& event) {
	g_settings.setInteger(Config::WELCOME_DIALOG, event.IsChecked() ? 1 : 0);
}

void WelcomeDialog::OnRecentFileClicked(wxCommandEvent& event) {
	wxCommandEvent newEvent(WELCOME_DIALOG_ACTION);
	newEvent.SetId(wxID_OPEN);
	newEvent.SetString(event.GetString());
	ProcessWindowEvent(newEvent);
}
