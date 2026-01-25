#include "ui/managers/recent_files_manager.h"

RecentFilesManager::RecentFilesManager() : recentFiles(10) {
}

RecentFilesManager::~RecentFilesManager() {
}

void RecentFilesManager::Load(wxConfigBase* config) {
	recentFiles.Load(*config);
}

void RecentFilesManager::Save(wxConfigBase* config) {
	recentFiles.Save(*config);
}

void RecentFilesManager::AddFile(const FileName& file) {
	recentFiles.AddFileToHistory(file.GetFullPath());
}

void RecentFilesManager::UseMenu(wxMenu* menu) {
	recentFiles.UseMenu(menu);
	recentFiles.AddFilesToMenu();
}

std::vector<wxString> RecentFilesManager::GetFiles() const {
	std::vector<wxString> files(recentFiles.GetCount());
	for (size_t i = 0; i < recentFiles.GetCount(); ++i) {
		files[i] = recentFiles.GetHistoryFile(i);
	}
	return files;
}

int RecentFilesManager::GetBaseId() const {
	return recentFiles.GetBaseId();
}

size_t RecentFilesManager::GetCount() const {
	return recentFiles.GetCount();
}

wxString RecentFilesManager::GetFile(size_t index) const {
	if (index < recentFiles.GetCount()) {
		return recentFiles.GetHistoryFile(index);
	}
	return "";
}
