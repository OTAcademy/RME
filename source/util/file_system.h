//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_UTIL_FILE_SYSTEM_H_
#define RME_UTIL_FILE_SYSTEM_H_

#include <wx/wx.h>

class FileSystem {
public:
	static wxString GetExecDirectory();
	static wxString GetDataDirectory();
	static wxString GetLocalDataDirectory();
	static wxString GetLocalDirectory();
	static wxString GetExtensionsDirectory();

	static void DiscoverDataDirectory(const wxString& existentFile);
	static wxString GetFoundDataDirectory();

private:
	static wxString m_dataDirectory;
};

#endif
