#include "app/main.h"
#include "ui/dialog_util.h"
#include "editor/editor_factory.h"
#include "editor/editor.h"
#include "editor/persistence/editor_persistence.h"
#include "app/managers/version_manager.h"
#include "ui/gui.h"
#include "app/settings.h"
#include "io/iomap.h"
#include "live/live_client.h"

void SetupCallbacks(Editor* editor) {
	editor->onStateChange = []() {
		g_gui.UpdateTitle();
		g_gui.UpdateMenus();
	};

	editor->selection.onSelectionChange = [](size_t count) {
		if (count > 0) {
			wxString ss;
			if (count == 1) {
				ss << "One tile selected.";
			} else {
				ss << count << " tiles selected.";
			}
			g_gui.SetStatusText(ss);
		} else {
			// Optional: Clear status text if nothing selected, or keep previous behavior
			// Previous behavior didn't clear it explicitly in updateSelectionCount if size <= 0
		}
	};
}

std::unique_ptr<Editor> EditorFactory::CreateEmpty(CopyBuffer& copybuffer) {
	ClientVersionID defaultVersion = ClientVersionID(g_settings.getInteger(Config::DEFAULT_CLIENT_VERSION));
	if (defaultVersion == CLIENT_VERSION_NONE) {
		defaultVersion = ClientVersion::getLatestVersion()->getID();
	}

	if (!EnsureVersion(defaultVersion)) {
		throw std::runtime_error("Couldn't load client version");
	}

	MapVersion mapVersion;
	mapVersion.otbm = g_version.GetCurrentVersion().getPrefferedMapVersionID();
	mapVersion.client = g_version.GetCurrentVersionID();

	std::unique_ptr<Editor> editor = std::make_unique<Editor>(copybuffer, mapVersion);
	SetupCallbacks(editor.get());
	return editor;
}

std::unique_ptr<Editor> EditorFactory::LoadFromFile(CopyBuffer& copybuffer, const FileName& fn) {
	// For loading, we might want to query version from headers first, OR assume current GUI version is what we want?
	// The original Editor constructor for file loading called EditorPersistence::loadMap.
	// We'll pass current GUI version as a default context, though EditorPersistence might override map.

	MapVersion mapVersion;
	mapVersion.otbm = g_version.GetCurrentVersion().getPrefferedMapVersionID();
	mapVersion.client = g_version.GetCurrentVersionID();

	std::unique_ptr<Editor> editor = std::make_unique<Editor>(copybuffer, mapVersion, fn);
	SetupCallbacks(editor.get());
	return editor;
}

std::unique_ptr<Editor> EditorFactory::JoinLive(CopyBuffer& copybuffer, std::unique_ptr<LiveClient> client) {
	MapVersion mapVersion;
	mapVersion.otbm = g_version.GetCurrentVersion().getPrefferedMapVersionID();
	mapVersion.client = g_version.GetCurrentVersionID();

	std::unique_ptr<Editor> editor = std::make_unique<Editor>(copybuffer, mapVersion, std::move(client));
	SetupCallbacks(editor.get());
	return editor;
}

bool EditorFactory::EnsureVersion(ClientVersionID version) {
	if (g_version.GetCurrentVersionID() == version) {
		return true;
	}

	if (!g_gui.CloseAllEditors()) {
		return false;
	}

	wxString error;
	wxArrayString warnings;
	if (g_version.LoadVersion(version, error, warnings)) {
		if (!warnings.IsEmpty()) {
			DialogUtil::ListDialog("Warnings", warnings);
		}
		return true;
	} else {
		DialogUtil::PopupDialog("Error", error, wxOK);
		return false;
	}
}
