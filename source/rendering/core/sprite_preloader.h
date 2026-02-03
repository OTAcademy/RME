#ifndef RME_RENDERING_CORE_SPRITE_PRELOADER_H_
#define RME_RENDERING_CORE_SPRITE_PRELOADER_H_

class Editor;
struct RenderView;
struct DrawingOptions;

/**
 * SpritePreloader provides utilities to warm up the texture atlas
 * before rendering starts. This avoids mid-frame stalls caused by
 * lazy loading of sprites (IO, decompression, and GPU upload).
 */
namespace SpritePreloader {
	/**
	 * Preload all sprites that are likely to be visible in the current view.
	 *
	 * @param editor The editor instance containing the map
	 * @param view The current viewport/render view
	 * @param options Drawing options (to determine visibility of layers, etc.)
	 */
	void PreloadVisibleSprites(Editor* editor, const RenderView& view, const DrawingOptions& options);
}

#endif
