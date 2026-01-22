//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "rendering/io/screenshot_saver.h"
#include "main.h" // For ASSERT and other common definitions if needed
#include <wx/wfstream.h>
#include <time.h>

wxString ScreenshotSaver::GenerateDateString() {
	time_t t = time(nullptr);
	struct tm* current_time = localtime(&t);
	ASSERT(current_time);

	wxString date;
	date << "screenshot_" << (1900 + current_time->tm_year);
	if (current_time->tm_mon < 9) {
		date << "-" << "0" << current_time->tm_mon + 1;
	} else {
		date << "-" << current_time->tm_mon + 1;
	}
	date << "-" << current_time->tm_mday;
	date << "-" << current_time->tm_hour;
	date << "-" << current_time->tm_min;
	date << "-" << current_time->tm_sec;
	return date;
}

wxString ScreenshotSaver::SaveScreenshot(const wxFileName& suggestedInfo, const wxString& format, int width, int height, uint8_t* buffer) {
	if (!buffer) {
		return "Error: Image capture failed. Old Video Driver?";
	}

	wxImage screenshot(width, height, buffer);
	wxFileName path = suggestedInfo;

	int type = 0;
	path.SetName(GenerateDateString());

	if (format == "bmp") {
		path.SetExt(format);
		type = wxBITMAP_TYPE_BMP;
	} else if (format == "png") {
		path.SetExt(format);
		type = wxBITMAP_TYPE_PNG;
	} else if (format == "jpg" || format == "jpeg") {
		path.SetExt(format);
		type = wxBITMAP_TYPE_JPEG;
	} else if (format == "tga") {
		path.SetExt(format);
		type = wxBITMAP_TYPE_TGA;
	} else {
		path.SetExt("png");
		type = wxBITMAP_TYPE_PNG;
	}

	if (!path.Mkdir(0755, wxPATH_MKDIR_FULL)) {
		// Just proceed and try saving, sometimes mkdir fails if it exists
	}

	wxFileOutputStream of(path.GetFullPath());
	if (of.IsOk()) {
		if (screenshot.SaveFile(of, static_cast<wxBitmapType>(type))) {
			return "Took screenshot and saved as " + path.GetFullName();
		} else {
			return "Error: Couldn't save image file correctly.";
		}
	} else {
		return "Error: Couldn't open file " + path.GetFullPath() + " for writing.";
	}
}
