#ifndef RME_RENDERING_GRID_DRAWER_H_
#define RME_RENDERING_GRID_DRAWER_H_

#include <wx/colour.h>

struct RenderView;
struct DrawingOptions;

class GridDrawer {
public:
	void DrawGrid(const RenderView& view, const DrawingOptions& options);
	void DrawIngameBox(const RenderView& view, const DrawingOptions& options);

private:
	void drawRect(int x, int y, int w, int h, const wxColor& color, int width = 1);
	void drawFilledRect(int x, int y, int w, int h, const wxColor& color);
};

#endif
