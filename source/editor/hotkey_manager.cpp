//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "editor/hotkey_manager.h"
#include "brushes/brush.h"
#include "app/settings.h"

HotkeyManager g_hotkeys;

Hotkey::Hotkey() {
	type = NONE;
}

Hotkey::Hotkey(Position p) {
	type = POSITION;
	pos = p;
}

Hotkey::Hotkey(Brush* b) {
	type = BRUSH;
	brushname = b->getName();
}

Hotkey::Hotkey(std::string b) {
	type = BRUSH;
	brushname = b;
}

Hotkey::~Hotkey() {
}

std::ostream& operator<<(std::ostream& os, const Hotkey& hotkey) {
	if (hotkey.type == Hotkey::POSITION) {
		os << "POS " << hotkey.pos;
	} else if (hotkey.type == Hotkey::BRUSH) {
		os << "BRUSH " << hotkey.brushname;
	} else {
		os << "NONE";
	}
	return os;
}

std::istream& operator>>(std::istream& os, Hotkey& hotkey) {
	std::string type_s;
	os >> type_s;
	if (type_s == "POS") {
		hotkey.type = Hotkey::POSITION;
		os >> hotkey.pos;
	} else if (type_s == "BRUSH") {
		hotkey.type = Hotkey::BRUSH;
		std::string brushname;
		std::getline(os >> std::ws, brushname);
		hotkey.brushname = brushname;
	} else {
		hotkey.type = Hotkey::NONE;
	}
	return os;
}

HotkeyManager::HotkeyManager() :
	hotkeys_enabled(true) {
}

HotkeyManager::~HotkeyManager() {
}

void HotkeyManager::EnableHotkeys() {
	hotkeys_enabled = true;
}

void HotkeyManager::DisableHotkeys() {
	hotkeys_enabled = false;
}

bool HotkeyManager::AreHotkeysEnabled() const {
	return hotkeys_enabled;
}

void HotkeyManager::SetHotkey(int index, Hotkey& hotkey) {
	if (index >= 0 && index < 10) {
		hotkeys[index] = hotkey;
	}
}

const Hotkey& HotkeyManager::GetHotkey(int index) const {
	if (index >= 0 && index < 10) {
		return hotkeys[index];
	}
	static Hotkey empty;
	return empty;
}

void HotkeyManager::SaveHotkeys() const {
	for (int i = 0; i < 10; ++i) {
		std::ostringstream ss;
		ss << hotkeys[i];
		g_settings.setString(Config::HOTKEY_0 + i, ss.str());
	}
}

void HotkeyManager::LoadHotkeys() {
	for (int i = 0; i < 10; ++i) {
		std::string str = g_settings.getString(Config::HOTKEY_0 + i);
		if (!str.empty()) {
			std::istringstream ss(str);
			ss >> hotkeys[i];
		}
	}
}
