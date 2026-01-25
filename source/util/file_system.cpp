//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "app/main.h"
#include "util/file_system.h"
// #include "utility/file_name.h"
#include "app/settings.h"
#include <wx/stdpaths.h>

wxString FileSystem::m_dataDirectory;

wxString FileSystem::GetDataDirectory() {
	std::string cfg_str = g_settings.getString(Config::DATA_DIRECTORY);
	if (!cfg_str.empty()) {
		FileName dir;
		dir.Assign(wxstr(cfg_str));
		wxString path;
		if (dir.DirExists()) {
			path = dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
			return path;
		}
	}

	// Silently reset directory
	FileName exec_directory;
	try {
		exec_directory = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetExecutablePath();
	} catch (const std::bad_cast) {
		throw; // Crash application (this should never happend anyways...)
	}

	exec_directory.AppendDir("data");
	return exec_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

wxString FileSystem::GetExecDirectory() {
	// Silently reset directory
	FileName exec_directory;
	try {
		exec_directory = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetExecutablePath();
	} catch (const std::bad_cast) {
		wxLogError("Could not fetch executable directory.");
	}
	return exec_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

wxString FileSystem::GetLocalDataDirectory() {
	if (g_settings.getInteger(Config::INDIRECTORY_INSTALLATION)) {
		FileName dir = GetDataDirectory();
		dir.AppendDir("user");
		dir.AppendDir("data");
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	} else {
		FileName dir = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetUserDataDir();
#ifdef __WINDOWS__
		dir.AppendDir("Remere's Map Editor");
#else
		dir.AppendDir(".rme");
#endif
		dir.AppendDir("data");
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	}
}

wxString FileSystem::GetLocalDirectory() {
	if (g_settings.getInteger(Config::INDIRECTORY_INSTALLATION)) {
		FileName dir = GetDataDirectory();
		dir.AppendDir("user");
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	} else {
		FileName dir = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetUserDataDir();
#ifdef __WINDOWS__
		dir.AppendDir("Remere's Map Editor");
#else
		dir.AppendDir(".rme");
#endif
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	}
}

wxString FileSystem::GetExtensionsDirectory() {
	std::string cfg_str = g_settings.getString(Config::EXTENSIONS_DIRECTORY);
	if (!cfg_str.empty()) {
		FileName dir;
		dir.Assign(wxstr(cfg_str));
		wxString path;
		if (dir.DirExists()) {
			path = dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
			return path;
		}
	}

	// Silently reset directory
	FileName local_directory = GetLocalDirectory();
	local_directory.AppendDir("extensions");
	local_directory.Mkdir(0755, wxPATH_MKDIR_FULL);
	return local_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

void FileSystem::DiscoverDataDirectory(const wxString& existentFile) {
	wxString currentDir = wxGetCwd();
	wxString execDir = GetExecDirectory();

	wxString possiblePaths[] = {
		execDir,
		currentDir + "/",

		// these are used usually when running from build directories
		execDir + "/../",
		execDir + "/../../",
		execDir + "/../../../",
		currentDir + "/../",
	};

	bool found = false;
	for (const wxString& path : possiblePaths) {
		if (wxFileName(path + "data/" + existentFile).FileExists()) {
			m_dataDirectory = path + "data/";
			found = true;
			break;
		}
	}

	if (!found) {
		wxLogError(wxString() + "Could not find data directory.\n");
	}
}

wxString FileSystem::GetFoundDataDirectory() {
	return m_dataDirectory;
}
