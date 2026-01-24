#ifndef RME_RENDERING_SELECTION_DRAWER_H_
#define RME_RENDERING_SELECTION_DRAWER_H_

struct RenderView;
class MapCanvas;
struct DrawingOptions;
class SpriteBatch;

class SelectionDrawer {
public:
	void draw(SpriteBatch& sprite_batch, const RenderView& view, const MapCanvas* canvas, const DrawingOptions& options);
};

#endif
