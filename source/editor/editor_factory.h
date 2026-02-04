#ifndef RME_EDITOR_FACTORY_H
#define RME_EDITOR_FACTORY_H
#include <memory>
#include "app/main.h"
#include "app/client_version.h"

class Editor;
class CopyBuffer;
class LiveClient;

class EditorFactory {
public:
	// Creates a new empty map, may prompt for version if not set
	static std::unique_ptr<Editor> CreateEmpty(CopyBuffer& copybuffer);

	// Loads a map from file, handles version checking/switching
	static std::unique_ptr<Editor> LoadFromFile(CopyBuffer& copybuffer, const FileName& fn);

	// Joins a live session
	static std::unique_ptr<Editor> JoinLive(CopyBuffer& copybuffer, std::unique_ptr<LiveClient> client);

private:
	// Helper to ensure the correct version is loaded in g_gui
	static bool EnsureVersion(ClientVersionID version);
};

#endif
