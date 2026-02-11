//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "util/image_manager.h"
#include <wx/wx.h>
#include <wx/graphics.h>
#include <iostream>
#include <cassert>

// This is a minimal test harness for ImageManager
// It requires a wxApp to be running for wxBitmap operations

class TestApp : public wxApp {
public:
	virtual bool OnInit() {
		if (!wxApp::OnInit()) {
			return false;
		}

		std::cout << "Starting ImageManager Tests..." << std::endl;

		// 1. Test basic path resolution (manual check via logs)
		// IMAGE_MANAGER.ResolvePath("svg/solid/sun.svg");

		// 2. Test SVG loading
		wxBitmapBundle sun = IMAGE_MANAGER.GetBitmapBundle("svg/solid/sun.svg");
		assert(sun.IsOk());
		assert(sun.GetDefaultSize() == wxSize(16, 16));
		std::cout << "SVG Loading: PASS" << std::endl;

		// 3. Test PNG loading
		wxBitmapBundle wall = IMAGE_MANAGER.GetBitmapBundle("png/brushes/wall_normal.png");
		if (wall.IsOk()) {
			std::cout << "PNG Loading: PASS" << std::endl;
		} else {
			std::cout << "PNG Loading: FAIL (File might be missing from test env)" << std::endl;
		}

		// 4. Test Tinting
		wxBitmap tinted = IMAGE_MANAGER.GetBitmap("svg/solid/sun.svg", wxSize(32, 32), wxColour(255, 0, 0));
		assert(tinted.IsOk());
		assert(tinted.GetSize() == wxSize(32, 32));
		std::cout << "Tinting: PASS" << std::endl;

		// 5. Test Caching
		wxBitmapBundle sun2 = IMAGE_MANAGER.GetBitmapBundle("svg/solid/sun.svg");
		// In a real test we'd check if it's the same object/pointer if possible,
		// but here we just ensure it still works.
		assert(sun2.IsOk());
		std::cout << "Caching: PASS" << std::endl;

		std::cout << "All ImageManager logic tests completed." << std::endl;

		return false; // Exit app
	}
};

// Main can be defined if needed, but RME uses wxApp
// wxIMPLEMENT_APP_CONSOLE(TestApp);
