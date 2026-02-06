#ifndef RME_IO_LOADERS_OTFI_LOADER_H_
#define RME_IO_LOADERS_OTFI_LOADER_H_

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filename.h>

class GraphicManager;

class OtfiLoader {
public:
	static bool Load(GraphicManager* manager, const wxFileName& filename, wxString& error, std::vector<std::string>& warnings);
};

#endif
