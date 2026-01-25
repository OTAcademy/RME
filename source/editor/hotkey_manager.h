//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_EDITOR_HOTKEY_MANAGER_H_
#define RME_EDITOR_HOTKEY_MANAGER_H_

#include "map/position.h"
#include <string>
#include <iostream>

class Brush;

class Hotkey {
public:
	Hotkey();
	Hotkey(Position pos);
	Hotkey(Brush* brush);
	Hotkey(std::string _brushname);
	~Hotkey();

	bool IsPosition() const {
		return type == POSITION;
	}
	bool IsBrush() const {
		return type == BRUSH;
	}
	Position GetPosition() const {
		return pos;
	}
	std::string GetBrushname() const {
		return brushname;
	}

private:
	enum {
		NONE,
		POSITION,
		BRUSH,
	} type;

	Position pos;
	std::string brushname;

	friend std::ostream& operator<<(std::ostream& os, const Hotkey& hotkey);
	friend std::istream& operator>>(std::istream& os, Hotkey& hotkey);
};

std::ostream& operator<<(std::ostream& os, const Hotkey& hotkey);
std::istream& operator>>(std::istream& os, Hotkey& hotkey);

class HotkeyManager {
public:
	HotkeyManager();
	~HotkeyManager();

	void EnableHotkeys();
	void DisableHotkeys();
	bool AreHotkeysEnabled() const;

	void SetHotkey(int index, Hotkey& hotkey);
	const Hotkey& GetHotkey(int index) const;
	void SaveHotkeys() const;
	void LoadHotkeys();

private:
	Hotkey hotkeys[10];
	bool hotkeys_enabled;
};

extern HotkeyManager g_hotkeys;

#endif
