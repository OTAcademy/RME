#ifndef RME_UI_GAMES_GAME_PANEL_H
#define RME_UI_GAMES_GAME_PANEL_H

#include <wx/wx.h>

class GamePanel : public wxPanel {
public:
	GamePanel(wxWindow* parent, int width, int height);
	virtual ~GamePanel();

	void OnPaint(wxPaintEvent&);
	void OnKeyDown(wxKeyEvent&);
	void OnKeyUp(wxKeyEvent&);
	void OnIdle(wxIdleEvent&);

	void pause() {
		paused_val = true;
	}
	void unpause() {
		paused_val = false;
	}
	bool paused() const {
		return paused_val || dead;
	}

protected:
	virtual void Render(wxDC& pdc) = 0;
	virtual void GameLoop(int time) = 0;
	virtual void OnKey(wxKeyEvent& event, bool down) = 0;

	virtual int getFPS() const = 0;

protected:
	wxStopWatch game_timer;

private:
	bool paused_val;

	DECLARE_EVENT_TABLE()

protected:
	bool dead;
};

#endif // RME_UI_GAMES_GAME_PANEL_H
