//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_VERSION_MANAGER_H_
#define RME_VERSION_MANAGER_H_

#include "app/main.h"
#include "app/client_version.h"

class VersionManager {
public:
	VersionManager();
	~VersionManager();

	void UnloadVersion();
	bool LoadVersion(ClientVersionID ver, wxString& error, std::vector<std::string>& warnings, bool force = false);

	// The current version loaded (returns CLIENT_VERSION_NONE if no version is loaded)
	const ClientVersion& GetCurrentVersion() const;
	ClientVersionID GetCurrentVersionID() const;

	// If any version is loaded at all
	bool IsVersionLoaded() const {
		return loaded_version != CLIENT_VERSION_NONE;
	}

	ClientVersion* getLoadedVersion() const {
		return loaded_version == CLIENT_VERSION_NONE ? nullptr : ClientVersion::get(loaded_version);
	}

private:
	bool LoadDataFiles(wxString& error, std::vector<std::string>& warnings);

	ClientVersionID loaded_version;
};

extern VersionManager g_version;

#endif
