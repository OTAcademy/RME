#ifndef WELCOME_DIALOG_H
#define WELCOME_DIALOG_H

#include <wx/wx.h>
#include <vector>

// Forward declarations
class ModernButton;

wxDECLARE_EVENT(WELCOME_DIALOG_ACTION, wxCommandEvent);

class WelcomeDialog : public wxDialog {
public:
	WelcomeDialog(const wxString& titleText, const wxString& versionText, const wxSize& size, const wxBitmap& rmeLogo, const std::vector<wxString>& recentFiles);

	// Event Handlers
	void OnButtonClicked(wxCommandEvent& event);
	void OnCheckboxClicked(wxCommandEvent& event);
	void OnRecentFileClicked(wxCommandEvent& event);

private:
	// Implementation details
	class WelcomePanel;
};

#endif // WELCOME_DIALOG_H
