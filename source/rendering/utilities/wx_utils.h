#ifndef RME_RENDERING_UTILITIES_WX_UTILS_H
#define RME_RENDERING_UTILITIES_WX_UTILS_H

#include <wx/bitmap.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <memory>

/**
 * Creates a wxBitmap from raw PNG data in memory.
 *
 * @param data Pointer to the raw bytes of a PNG image.
 * @param length Size of the data in bytes.
 * @return A unique_ptr to the created wxBitmap, or nullptr if creation failed.
 */
inline std::unique_ptr<wxBitmap> _wxGetBitmapFromMemory(const unsigned char* data, int length) {
	wxMemoryInputStream is(data, length);
	wxImage img(is, "image/png");
	if (!img.IsOk()) {
		return nullptr;
	}
	return std::make_unique<wxBitmap>(img, -1);
}

#endif // RME_RENDERING_UTILITIES_WX_UTILS_H
