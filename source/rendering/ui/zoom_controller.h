//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_ZOOM_CONTROLLER_H
#define RME_ZOOM_CONTROLLER_H

class MapCanvas;
class wxMouseEvent;

class ZoomController {
public:
	static void OnWheel(MapCanvas* canvas, wxMouseEvent& event);
	static void SetZoom(MapCanvas* canvas, double value);
	static void ApplyRelativeZoom(MapCanvas* canvas, double diff);
	static void UpdateStatus(MapCanvas* canvas);
};

#endif
