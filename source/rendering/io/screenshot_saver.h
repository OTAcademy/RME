//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_SCREENSHOT_SAVER_H_
#define RME_RENDERING_SCREENSHOT_SAVER_H_

#include <wx/string.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/bitmap.h>

class ScreenshotSaver {
public:
	// Returns status message or error message
	static wxString SaveScreenshot(const wxFileName& path, const wxString& format, int width, int height, uint8_t* buffer);

private:
	static wxString GenerateDateString();
};

#endif
