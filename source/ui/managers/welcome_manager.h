#ifndef RME_WELCOME_MANAGER_H_
#define RME_WELCOME_MANAGER_H_

#include <wx/wx.h>

class WelcomeDialog;

class WelcomeManager {
public:
	WelcomeManager();
	~WelcomeManager();

	void ShowWelcomeDialog(const wxBitmap& icon);
	void FinishWelcomeDialog();
	bool IsWelcomeDialogShown();

	void OnWelcomeDialogClosed(wxCloseEvent& event);
	void OnWelcomeDialogAction(wxCommandEvent& event);

private:
	WelcomeDialog* welcomeDialog;
};

extern WelcomeManager g_welcome;

#endif
