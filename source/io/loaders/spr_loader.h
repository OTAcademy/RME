#ifndef RME_IO_LOADERS_SPR_LOADER_H_
#define RME_IO_LOADERS_SPR_LOADER_H_

#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <memory>
#include <cstdint>

class GraphicManager;
class FileReadHandle;

class SprLoader {
public:
	static bool LoadData(GraphicManager* manager, const wxFileName& datafile, wxString& error, wxArrayString& warnings);
	static bool LoadDump(GraphicManager* manager, std::unique_ptr<uint8_t[]>& target, uint16_t& size, int sprite_id);

private:
	static std::vector<uint32_t> ReadSpriteIndexes(FileReadHandle& fh, uint32_t total_pics, wxString& error);
	static bool ReadSprites(GraphicManager* manager, FileReadHandle& fh, const std::vector<uint32_t>& sprite_indexes, wxArrayString& warnings, wxString& error);
};

#endif
