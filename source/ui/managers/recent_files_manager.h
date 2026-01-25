#ifndef RME_RECENT_FILES_MANAGER_H_
#define RME_RECENT_FILES_MANAGER_H_

#include "app/main.h"
#include <wx/docview.h>
#include <wx/fileconf.h>
#include <vector>

class RecentFilesManager {
public:
	RecentFilesManager();
	~RecentFilesManager();

	void Load(wxConfigBase* config);
	void Save(wxConfigBase* config);
	void AddFile(const FileName& file);
	void UseMenu(wxMenu* menu);

	std::vector<wxString> GetFiles() const;
	int GetBaseId() const;
	size_t GetCount() const;
	wxString GetFile(size_t index) const;

private:
	wxFileHistory recentFiles;
};

#endif
