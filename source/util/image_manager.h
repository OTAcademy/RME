//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_IMAGE_MANAGER_H_
#define RME_IMAGE_MANAGER_H_

#include <wx/wx.h>
#include <wx/bmpbndl.h>
#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <map>
#include <string>
#include <memory>
#include <cstdint>
#include <vector>
#include <utility>

struct NVGcontext;

class ImageManager {
public:
	static ImageManager& GetInstance();

	// wxWidgets support
	wxBitmapBundle GetBitmapBundle(const std::string& assetPath, const wxColour& tint = wxNullColour);
	wxBitmap GetBitmap(const std::string& assetPath, const wxSize& size = wxDefaultSize, const wxColour& tint = wxNullColour);

	// NanoVG support
	int GetNanoVGImage(NVGcontext* vg, const std::string& assetPath);

	// OpenGL support
	uint32_t GetGLTexture(const std::string& assetPath);

	// Cleanup
	void ClearCache();

private:
	ImageManager();
	~ImageManager();

	std::string ResolvePath(const std::string& assetPath);

	// Caches
	std::map<std::string, wxBitmapBundle> m_bitmapBundleCache;
	std::map<std::pair<std::string, uint32_t>, wxBitmap> m_tintedBitmapCache;
	std::map<std::string, int> m_nvgImageCache;
	std::map<std::string, uint32_t> m_glTextureCache;

	// Helper for tinting
	wxImage TintImage(const wxImage& image, const wxColour& tint);
};

// Helper macros for common assets
#define IMAGE_MANAGER ImageManager::GetInstance()

// Shortcut macros - Single Source of Truth for all asset paths
// Common Icons
#define ICON_SUNNY "svg/solid/sun.svg"
#define ICON_LOCATION "svg/solid/location-crosshairs.svg"
#define ICON_ACCOUNT "svg/solid/circle-user.svg"
#define ICON_MINUS "svg/solid/circle-minus.svg"
#define ICON_PLUS "svg/solid/circle-plus.svg"
#define ICON_LOCATION_ARROW "svg/solid/location-arrow.svg"

// Standard Actions
#define ICON_NEW "svg/regular/file.svg"
#define ICON_OPEN "svg/regular/folder-open.svg"
#define ICON_SAVE "svg/regular/floppy-disk.svg"
#define ICON_UNDO "svg/solid/undo.svg"
#define ICON_REDO "svg/solid/redo.svg"
#define ICON_CUT "svg/solid/scissors.svg"
#define ICON_COPY "svg/regular/copy.svg"
#define ICON_PASTE "svg/regular/paste.svg"
#define ICON_FIND "svg/solid/magnifying-glass.svg"

// Brush Assets (PNGs)
#define IMAGE_CIRCULAR_1 "png/circular_1.png"
#define IMAGE_CIRCULAR_2 "png/circular_2.png"
#define IMAGE_CIRCULAR_3 "png/circular_3.png"
#define IMAGE_CIRCULAR_4 "png/circular_4.png"
#define IMAGE_CIRCULAR_5 "png/circular_5.png"
#define IMAGE_CIRCULAR_6 "png/circular_6.png"
#define IMAGE_CIRCULAR_7 "png/circular_7.png"
#define IMAGE_CIRCULAR_1_SMALL "png/circular_1_small.png"
#define IMAGE_CIRCULAR_2_SMALL "png/circular_2_small.png"
#define IMAGE_CIRCULAR_3_SMALL "png/circular_3_small.png"
#define IMAGE_CIRCULAR_4_SMALL "png/circular_4_small.png"
#define IMAGE_CIRCULAR_5_SMALL "png/circular_5_small.png"
#define IMAGE_CIRCULAR_6_SMALL "png/circular_6_small.png"
#define IMAGE_CIRCULAR_7_SMALL "png/circular_7_small.png"

#define IMAGE_RECTANGULAR_1 "png/rectangular_1.png"
#define IMAGE_RECTANGULAR_2 "png/rectangular_2.png"
#define IMAGE_RECTANGULAR_3 "png/rectangular_3.png"
#define IMAGE_RECTANGULAR_4 "png/rectangular_4.png"
#define IMAGE_RECTANGULAR_5 "png/rectangular_5.png"
#define IMAGE_RECTANGULAR_6 "png/rectangular_6.png"
#define IMAGE_RECTANGULAR_7 "png/rectangular_7.png"
#define IMAGE_RECTANGULAR_1_SMALL "png/rectangular_1_small.png"
#define IMAGE_RECTANGULAR_2_SMALL "png/rectangular_2_small.png"
#define IMAGE_RECTANGULAR_3_SMALL "png/rectangular_3_small.png"
#define IMAGE_RECTANGULAR_4_SMALL "png/rectangular_4_small.png"
#define IMAGE_RECTANGULAR_5_SMALL "png/rectangular_5_small.png"
#define IMAGE_RECTANGULAR_6_SMALL "png/rectangular_6_small.png"
#define IMAGE_RECTANGULAR_7_SMALL "png/rectangular_7_small.png"

#define IMAGE_PROTECTION_ZONE "png/protection_zone.png"
#define IMAGE_PROTECTION_ZONE_SMALL "png/protection_zone_small.png"
#define IMAGE_PVP_ZONE "png/pvp_zone.png"
#define IMAGE_PVP_ZONE_SMALL "png/pvp_zone_small.png"
#define IMAGE_NO_PVP_ZONE "png/no_pvp.png"
#define IMAGE_NO_PVP_ZONE_SMALL "png/no_pvp_small.png"
#define IMAGE_NO_LOGOUT_ZONE "png/no_logout.png"
#define IMAGE_NO_LOGOUT_ZONE_SMALL "png/no_logout_small.png"
#define IMAGE_OPTIONAL_BORDER "png/optional_border.png"
#define IMAGE_OPTIONAL_BORDER_SMALL "png/optional_border_small.png"
#define IMAGE_ERASER "png/eraser.png"
#define IMAGE_ERASER_SMALL "png/eraser_small.png"

#define IMAGE_DOOR_NORMAL "png/door_normal.png"
#define IMAGE_DOOR_NORMAL_SMALL "png/door_normal_small.png"
#define IMAGE_DOOR_LOCKED "png/door_locked.png"
#define IMAGE_DOOR_LOCKED_SMALL "png/door_locked_small.png"
#define IMAGE_DOOR_MAGIC "png/door_magic.png"
#define IMAGE_DOOR_MAGIC_SMALL "png/door_magic_small.png"
#define IMAGE_DOOR_QUEST "png/door_quest.png"
#define IMAGE_DOOR_QUEST_SMALL "png/door_quest_small.png"
#define IMAGE_DOOR_NORMAL_ALT "png/door_normal_alt.png"
#define IMAGE_DOOR_NORMAL_ALT_SMALL "png/door_normal_alt_small.png"
#define IMAGE_DOOR_ARCHWAY "png/door_archway.png"
#define IMAGE_DOOR_ARCHWAY_SMALL "png/door_archway_small.png"

#define IMAGE_WINDOW_NORMAL "png/window_normal.png"
#define IMAGE_WINDOW_NORMAL_SMALL "png/window_normal_small.png"
#define IMAGE_WINDOW_HATCH "png/window_hatch.png"
#define IMAGE_WINDOW_HATCH_SMALL "png/window_hatch_small.png"

#define IMAGE_GEM_EDIT "png/gem_edit.png"
#define IMAGE_GEM_MOVE "png/gem_move.png"

#endif
