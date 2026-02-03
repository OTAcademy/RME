#ifndef RME_IO_LOADERS_SPR_LOADER_H_
#define RME_IO_LOADERS_SPR_LOADER_H_

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <memory>
#include <cstdint>

class GraphicManager;

class SprLoader {
public:
	static bool LoadData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings);
	static bool LoadDump(GraphicManager* manager, std::unique_ptr<uint8_t[]>& target, uint16_t& size, int sprite_id);
};

#endif
