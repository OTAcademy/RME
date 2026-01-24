//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_SCREENSHOT_CONTROLLER_H
#define RME_SCREENSHOT_CONTROLLER_H

#include <wx/wx.h>
#include <wx/filename.h>
#include <memory>

class MapCanvas;
class ScreenshotSaver;

class ScreenshotController {
public:
	ScreenshotController(MapCanvas* canvas);
	~ScreenshotController();

	void TakeScreenshot(const wxFileName& path, const wxString& format);
	bool IsCapturing() const;
	uint8_t* GetBuffer();

private:
	MapCanvas* canvas;
	std::unique_ptr<ScreenshotSaver> screenshot_saver;
};

#endif
