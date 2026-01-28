//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "app/main.h"

#include "ui/gui.h"
#include "ui/dialog_util.h"

#include "ui/about_window.h"
#include "ui/games/game_panel.h"
#include "ui/games/tetris_panel.h"
#include "ui/games/snake_panel.h"
#include <fstream>
#include <typeinfo>
#include <memory>

//=============================================================================
// About Window - Information window about the application

BEGIN_EVENT_TABLE(AboutWindow, wxDialog)
EVT_BUTTON(wxID_OK, AboutWindow::OnClickOK)
EVT_BUTTON(ABOUT_VIEW_LICENSE, AboutWindow::OnClickLicense)
EVT_MENU(ABOUT_RUN_TETRIS, AboutWindow::OnTetris)
EVT_MENU(ABOUT_RUN_SNAKE, AboutWindow::OnSnake)
EVT_MENU(wxID_CANCEL, AboutWindow::OnClickOK)
END_EVENT_TABLE()

AboutWindow::AboutWindow(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "About", wxDefaultPosition, wxSize(300, 320), wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX),
	game_panel(nullptr) {
	wxString about;

	about << "OTAcademy Map Editor\n";
	about << "(based on Remere's Map Editor)\n\n";
	about << "This program is a map editor for game servers\nthat derivied from OpenTibia project.\n\n";
	about << "Brought to you by OTAcademy\n\n";

	about << "Version " << __W_RME_VERSION__ << " for ";
	about <<
#ifdef __WINDOWS__
		"Windows";
#elif __LINUX__
		"Linux";
#elif __APPLE__
		"macOS";
#else
		"other OS";
#endif
	about << "\n\n";

	about << "Using " << wxVERSION_STRING << " interface\n";
	about << "OpenGL version " << wxString((char*)glGetString(GL_VERSION), wxConvUTF8) << "\n";
	about << "\n";
	about << "This program comes with ABSOLUTELY NO WARRANTY;\n";
	about << "for details see the LICENSE file.\n";
	about << "This is free software, and you are welcome to redistribute it\n";
	about << "under certain conditions.\n";
	about << "\n";
	about << "Compiled on: " << __TDATE__ << " : " << __TTIME__ << "\n";
	about << "Compiled with: " << BOOST_COMPILER << "\n";

	topsizer = newd wxBoxSizer(wxVERTICAL);

	topsizer->Add(newd wxStaticText(this, wxID_ANY, about), 1, wxALL, 20);

	wxSizer* choicesizer = newd wxBoxSizer(wxHORIZONTAL);
	choicesizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 20);

	wxAcceleratorEntry entries[3];
	entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
	entries[1].Set(wxACCEL_NORMAL, 't', ABOUT_RUN_TETRIS);
	entries[2].Set(wxACCEL_NORMAL, 's', ABOUT_RUN_SNAKE);
	wxAcceleratorTable accel(3, entries);
	SetAcceleratorTable(accel);

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

AboutWindow::~AboutWindow() {
	////
}

void AboutWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}

void AboutWindow::OnClickLicense(wxCommandEvent& WXUNUSED(event)) {
	FileName path;
	try {
		path = wxStandardPaths::Get().GetExecutablePath();
	} catch (std::bad_cast&) {
		return;
	}
	path.SetFullName("COPYING.txt");
	std::ifstream gpl(path.GetFullPath().mb_str());

	std::string gpl_str;
	char ch;
	while (gpl.get(ch)) {
		gpl_str += ch;
	}

	DialogUtil::ShowTextBox(this, "License", wxstr(gpl_str.size() ? gpl_str : "The COPYING.txt file is not available."));
}

void AboutWindow::OnTetris(wxCommandEvent&) {
	if (!game_panel) {
		DestroyChildren();
		game_panel = newd TetrisPanel(this);
		topsizer->Add(game_panel, 1, wxALIGN_CENTER | wxALL, 7);
		Fit();
		game_panel->SetFocus();
		SetWindowStyleFlag(wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
		Refresh();
	}
}

void AboutWindow::OnSnake(wxCommandEvent&) {
	if (!game_panel) {
		DestroyChildren();
		game_panel = newd SnakePanel(this);
		topsizer->Add(game_panel, 1, wxALIGN_CENTER | wxALL, 7);
		Fit();
		game_panel->SetFocus();
		SetWindowStyleFlag(wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
		Refresh();
	}
}
