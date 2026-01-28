#include "app/main.h"
#include "ui/games/game_panel.h"
#include <wx/dcbuffer.h>
#include <wx/dialog.h>

//=============================================================================
// GamePanel - Abstract class for games

BEGIN_EVENT_TABLE(GamePanel, wxPanel)
EVT_KEY_DOWN(GamePanel::OnKeyDown)
EVT_KEY_UP(GamePanel::OnKeyUp)
EVT_PAINT(GamePanel::OnPaint)
EVT_IDLE(GamePanel::OnIdle)
END_EVENT_TABLE()

GamePanel::GamePanel(wxWindow* parent, int width, int height) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(width, height), wxWANTS_CHARS),
	paused_val(false),
	dead(false) {
	// Receive idle events
	SetExtraStyle(wxWS_EX_PROCESS_IDLE);
	// Complete redraw
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

GamePanel::~GamePanel() {
	////
}

void GamePanel::OnPaint(wxPaintEvent&) {
	wxBufferedPaintDC pdc(this);
	Render(pdc);
}

void GamePanel::OnKeyDown(wxKeyEvent& event) {
	switch (event.GetKeyCode()) {
		case WXK_ESCAPE: {
			if (dead) {
				return;
			}
			wxDialog* dlg = (wxDialog*)GetParent();
			dlg->EndModal(0);
			break;
		}
		default: {
			OnKey(event, true);
			break;
		}
	}
}

void GamePanel::OnKeyUp(wxKeyEvent& event) {
	OnKey(event, false);
}

void GamePanel::OnIdle(wxIdleEvent& event) {
	int time = game_timer.Time();
	if (time > 1000 / getFPS()) {
		game_timer.Start();
		if (!paused()) {
			GameLoop(time);
		}
	}
	if (!paused()) {
		event.RequestMore(true);
	}
}
