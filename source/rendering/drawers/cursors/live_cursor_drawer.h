#ifndef RME_RENDERING_LIVE_CURSOR_DRAWER_H_
#define RME_RENDERING_LIVE_CURSOR_DRAWER_H_

struct RenderView;
class Editor;
class LiveSocket;

struct DrawingOptions;

class LiveCursorDrawer {
public:
	void draw(const RenderView& view, Editor& editor, const DrawingOptions& options);
};

#endif
