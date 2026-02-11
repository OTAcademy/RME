#ifndef RME_UTIL_NANOVG_CANVAS_H_
#define RME_UTIL_NANOVG_CANVAS_H_

#include "app/main.h"

#include <unordered_map>
#include <list>
#include <cstdint>
#include <memory>

#include "rendering/core/graphics.h"

struct NVGcontext;

/**
 * @class NanoVGCanvas
 * @brief Base class for wxGLCanvas controls that use NanoVG for hardware-accelerated 2D rendering.
 *
 * This class provides a standardized pattern for creating high-performance, custom-drawn
 * UI controls using NanoVG. It handles OpenGL context management, NanoVG initialization,
 * texture caching, and virtual scrolling.
 *
 * ## Usage
 * Subclass NanoVGCanvas and override:
 * - `OnNanoVGPaint()` - Your custom drawing logic
 * - `DoGetBestClientSize()` - Return preferred control size
 *
 * ## Example
 * ```cpp
 * class MyGridPanel : public NanoVGCanvas {
 * protected:
 *     void OnNanoVGPaint(NVGcontext* vg, int width, int height) override {
 *         nvgBeginPath(vg);
 *         nvgRoundedRect(vg, 10, 10, 100, 50, 4.0f);
 *         nvgFillColor(vg, nvgRGBA(80, 80, 80, 255));
 *         nvgFill(vg);
 *     }
 * };
 * ```
 */
/**
 * @class ScopedGLContext
 * @brief RAII helper to ensure a NanoVGCanvas's OpenGL context is current.
 */
class ScopedGLContext {
public:
	explicit ScopedGLContext(class NanoVGCanvas* canvas);
	~ScopedGLContext() = default; // Restoration of previous context is not reliably possible in wx without a global tracker

private:
	class NanoVGCanvas* m_canvas;
};

class NanoVGCanvas : public wxGLCanvas {
	friend class ScopedGLContext;

public:
	/**
	 * @brief Constructs a NanoVGCanvas control.
	 * @param parent Parent window
	 * @param id Window ID (default: wxID_ANY)
	 * @param style Additional window styles (default includes vertical scrollbar)
	 */
	NanoVGCanvas(wxWindow* parent, wxWindowID id = wxID_ANY, long style = wxVSCROLL | wxWANTS_CHARS);

	/**
	 * @brief Destructor - cleans up OpenGL and NanoVG resources.
	 */
	virtual ~NanoVGCanvas();

	// Non-copyable
	NanoVGCanvas(const NanoVGCanvas&) = delete;
	NanoVGCanvas& operator=(const NanoVGCanvas&) = delete;

	/**
	 * @brief Gets the current scroll position in pixels.
	 * @return Current vertical scroll offset
	 */
	[[nodiscard]] int GetScrollPosition() const {
		return m_scrollPos;
	}

	/**
	 * @brief Sets the scroll position.
	 * @param pos New scroll position in pixels
	 */
	void SetScrollPosition(int pos);

	/**
	 * @brief Gets the NanoVG context (only valid after first paint).
	 * @return NanoVG context pointer, or nullptr if not initialized
	 */
	[[nodiscard]] NVGcontext* GetNVGContext() const {
		return m_nvg.get();
	}

	int GetOrCreateItemImage(uint16_t itemId);

	/**
	 * @brief Gets or creates a cached NanoVG image for a static asset.
	 */
	int GetOrCreateStaticImage(const std::string& assetPath);

	/**
	 * @brief Gets or creates a cached NanoVG image for a Sprite (GameSprite or generic).
	 */
	int GetOrCreateSpriteTexture(NVGcontext* vg, Sprite* sprite);

protected:
	/**
	 * @brief Override this to implement your custom NanoVG drawing.
	 * @param vg The NanoVG context
	 * @param width Canvas width in pixels
	 * @param height Canvas height in pixels
	 *
	 * The coordinate system is already translated by -scrollPos on Y axis.
	 * Use GetScrollPosition() if you need the raw scroll value.
	 */
	virtual void OnNanoVGPaint(NVGcontext* vg, int width, int height) = 0;

	/**
	 * @brief Override to return preferred control size.
	 * @return Best client size for sizer calculations
	 */
	virtual wxSize DoGetBestClientSize() const override;

	/**
	 * @brief Creates or retrieves a cached NanoVG image from RGBA data.
	 * @param id Unique identifier for the image (e.g., item ID)
	 * @param data RGBA pixel data
	 * @param width Image width
	 * @param height Image height
	 * @return NanoVG image handle, or 0 on failure
	 */
	int GetOrCreateImage(uint64_t id, const uint8_t* data, int width, int height);
	int CreateGameSpriteTexture(NVGcontext* vg, GameSprite* gs, uint64_t spriteId);
	int CreateGenericSpriteTexture(NVGcontext* vg, Sprite* sprite, uint64_t spriteId);

	/**
	 * @brief Deletes a cached image.
	 * @param id Image identifier to delete
	 */
	void DeleteCachedImage(uint64_t id);

	/**
	 * @brief Adds an externally created image to the cache.
	 * @param id Unique identifier
	 * @param imageHandle NanoVG image handle
	 */
	void AddCachedImage(uint64_t id, int imageHandle);

	/**
	 * @brief Clears all cached images.
	 * Call this when the data set changes entirely.
	 */
	void ClearImageCache();

	/**
	 * @brief Checks if an image is already cached.
	 * @param id Image identifier
	 * @return Cached image handle, or 0 if not cached
	 */
	[[nodiscard]] int GetCachedImage(uint64_t id) const;

	/**
	 * @brief Updates the scrollbar based on content size.
	 * @param contentHeight Total height of content in pixels
	 */
	void UpdateScrollbar(int contentHeight);

	/**
	 * @brief Makes the GL context current. Call before any GL operations.
	 * @return true if context is valid and made current
	 */
	bool MakeContextCurrent();

public: // Public for ScopedGLContext and future extensions
	// Background color (can be overridden by subclasses)
	float m_bgRed = 45.0f / 255.0f;
	float m_bgGreen = 45.0f / 255.0f;
	float m_bgBlue = 45.0f / 255.0f;

private:
	void InitGL();
	void OnPaint(wxPaintEvent& evt);
	void OnSize(wxSizeEvent& evt);
	void OnMouseWheel(wxMouseEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt);
	void OnScroll(wxScrollWinEvent& evt);

	std::unique_ptr<wxGLContext> m_glContext;
	std::unique_ptr<NVGcontext, NVGDeleter> m_nvg;
	bool m_glInitialized = false;

	// Texture cache: ID -> NanoVG image handle
	std::unordered_map<uint64_t, int> m_imageCache;
	mutable std::list<uint64_t> m_lruList;
	size_t m_maxCacheSize = 1024; // Default limit

	// Scroll state
	int m_scrollPos = 0;
	int m_contentHeight = 0;
	int m_scrollStep = 40; // Pixels per wheel notch

protected:
	/**
	 * @brief Sets the scroll step (pixels per wheel notch).
	 * @param step Pixels to scroll per wheel event
	 */
	void SetScrollStep(int step) {
		m_scrollStep = step;
	}
};

#endif // RME_UTIL_NANOVG_CANVAS_H_
