//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_NAVIGATION_CONTROLLER_H
#define RME_NAVIGATION_CONTROLLER_H

class MapCanvas;
class wxMouseEvent;
class wxKeyEvent;

class NavigationController {
public:
	static void HandleArrowKeys(MapCanvas* canvas, wxKeyEvent& event);
	static void HandleMouseDrag(MapCanvas* canvas, wxMouseEvent& event);
	static void HandleCameraClick(MapCanvas* canvas, wxMouseEvent& event);
	static void HandleCameraRelease(MapCanvas* canvas, wxMouseEvent& event);
	static void HandleWheel(MapCanvas* canvas, wxMouseEvent& event);
	static void ChangeFloor(MapCanvas* canvas, int new_floor);
};

#endif
