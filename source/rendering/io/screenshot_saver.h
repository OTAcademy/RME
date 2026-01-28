//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_RENDERING_SCREENSHOT_SAVER_H_
#define RME_RENDERING_SCREENSHOT_SAVER_H_

#include <wx/string.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <cstdint>

class ScreenshotSaver {
public:
	ScreenshotSaver() :
		buffer(nullptr) { }
	~ScreenshotSaver() {
		Cleanup();
	}

	void PrepareCapture(int width, int height);
	wxString SaveCapture(const wxFileName& path, const wxString& format, int width, int height);

	uint8_t* GetBuffer() const {
		return buffer;
	}
	bool IsCapturing() const {
		return buffer != nullptr;
	}

	void Cleanup();

private:
	uint8_t* buffer;

	static wxString GenerateDateString();
};

#endif
