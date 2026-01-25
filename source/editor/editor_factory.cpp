#include "app/main.h"
#include "editor/editor_factory.h"
#include "editor/editor.h"
#include "editor/persistence/editor_persistence.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "io/iomap.h"

Editor* EditorFactory::CreateEmpty(CopyBuffer& copybuffer) {
	ClientVersionID defaultVersion = ClientVersionID(g_settings.getInteger(Config::DEFAULT_CLIENT_VERSION));
	if (defaultVersion == CLIENT_VERSION_NONE) {
		defaultVersion = ClientVersion::getLatestVersion()->getID();
	}

	if (!EnsureVersion(defaultVersion)) {
		throw std::runtime_error("Couldn't load client version");
	}

	return newd Editor(copybuffer);
}

Editor* EditorFactory::LoadFromFile(CopyBuffer& copybuffer, const FileName& fn) {
	// Editor constructor for File calls EditorPersistence::loadMap which handles version checking
	// We might want to move that logic here later, but for now let's keep it consistent
	return newd Editor(copybuffer, fn);
}

Editor* EditorFactory::JoinLive(CopyBuffer& copybuffer, LiveClient* client) {
	return newd Editor(copybuffer, client);
}

bool EditorFactory::EnsureVersion(ClientVersionID version) {
	if (g_gui.GetCurrentVersionID() == version) {
		return true;
	}

	if (!g_gui.CloseAllEditors()) {
		return false;
	}

	wxString error;
	wxArrayString warnings;
	if (g_gui.LoadVersion(version, error, warnings)) {
		if (!warnings.IsEmpty()) {
			g_gui.ListDialog("Warnings", warnings);
		}
		return true;
	} else {
		g_gui.PopupDialog("Error", error, wxOK);
		return false;
	}
}
