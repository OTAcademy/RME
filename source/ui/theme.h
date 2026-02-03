#ifndef RME_THEME_H_
#define RME_THEME_H_

#include <wx/wx.h>
#include <wx/settings.h>

class Theme {
public:
	enum class Role {
		Surface, // Main panel background
		Background, // Darker backgrounds (grids, lists)
		Header, // Even darker for headers
		Accent, // Primary action / Highlight (Blue)
		AccentHover, // Lighter accent
		Text, // Primary text
		TextSubtle, // De-emphasized text
		Border, // Outline/Separator colors
		Selected, // Selection fill
		Error // Warning/Error states
	};

	static wxColour Get(Role role) {
		switch (role) {
			case Role::Surface:
				return wxColour(45, 45, 48);
			case Role::Background:
				return wxColour(30, 30, 30);
			case Role::Header:
				return wxColour(25, 25, 25);
			case Role::Accent:
				return wxColour(0, 120, 215);
			case Role::AccentHover:
				return wxColour(0, 150, 255);
			case Role::Text:
				return wxColour(230, 230, 230);
			case Role::TextSubtle:
				return wxColour(150, 150, 150);
			case Role::Border:
				return wxColour(60, 60, 60);
			case Role::Selected:
				return wxColour(0, 120, 215, 60); // Alpha translucency
			case Role::Error:
				return wxColour(200, 50, 50);
			default:
				return *wxWHITE;
		}
	}

	static int Grid(int units) {
		return wxWindow::FromDIP(units * 4, nullptr);
	}

	static wxFont GetFont(int pointSize = 9, bool bold = false) {
		return wxFont(wxFontInfo(wxWindow::FromDIP(pointSize, nullptr))
						  .FaceName("Segoe UI")
						  .Bold(bold));
	}
};

#endif
