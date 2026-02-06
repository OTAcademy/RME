#include "io/loaders/otfi_loader.h"

#include "rendering/core/graphics.h"
#include "map/otml.h"
#include <wx/dir.h>
#include "app/main.h" // For ASSETS_NAME

bool OtfiLoader::Load(GraphicManager* manager, const wxFileName& filename, wxString& error, std::vector<std::string>& warnings) {
	// Set default values first
	manager->is_extended = false;
	manager->has_transparency = false;
	manager->has_frame_durations = false;
	manager->has_frame_groups = false;
	manager->metadata_file = wxFileName(filename.GetFullPath(), wxString(ASSETS_NAME) + ".dat");
	manager->sprites_file = wxFileName(filename.GetFullPath(), wxString(ASSETS_NAME) + ".spr");
	manager->otfi_found = false;

	wxDir dir(filename.GetFullPath());
	wxString otfi_file;

	if (dir.GetFirst(&otfi_file, "*.otfi", wxDIR_FILES)) {
		wxFileName otfi(filename.GetFullPath(), otfi_file);
		OTMLDocumentPtr doc = OTMLDocument::parse(otfi.GetFullPath().ToStdString());

		if (doc->size() == 0 || !doc->hasChildAt("DatSpr")) {
			error += "'DatSpr' tag not found";
			return false;
		}

		OTMLNodePtr node = doc->get("DatSpr");
		manager->is_extended = node->valueAt<bool>("extended");
		manager->has_transparency = node->valueAt<bool>("transparency");
		manager->has_frame_durations = node->valueAt<bool>("frame-durations");
		manager->has_frame_groups = node->valueAt<bool>("frame-groups");

		std::string metadata = node->valueAt<std::string>("metadata-file", std::string(ASSETS_NAME) + ".dat");
		std::string sprites = node->valueAt<std::string>("sprites-file", std::string(ASSETS_NAME) + ".spr");

		manager->metadata_file = wxFileName(filename.GetFullPath(), wxString(metadata));
		manager->sprites_file = wxFileName(filename.GetFullPath(), wxString(sprites));
		manager->otfi_found = true;
	}

	return true;
}
