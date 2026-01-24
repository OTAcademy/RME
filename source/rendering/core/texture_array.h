#ifndef RME_RENDERING_CORE_TEXTURE_ARRAY_H_
#define RME_RENDERING_CORE_TEXTURE_ARRAY_H_

#include "app/main.h"
#include <vector>

/**
 * OpenGL Texture Array (GL_TEXTURE_2D_ARRAY) for sprite atlas.
 *
 * All game sprites are stored in layers of this array, eliminating
 * texture switching during rendering.
 *
 * Each layer is a fixed-size 2D texture (e.g., 32x32 for sprites).
 * Sprites get a layer_index which becomes the Z coordinate in sampling.
 */
class TextureArray {
public:
	TextureArray();
	~TextureArray();

	// Non-copyable
	TextureArray(const TextureArray&) = delete;
	TextureArray& operator=(const TextureArray&) = delete;

	/**
	 * Initialize the texture array.
	 * @param width Width of each layer (e.g., 32)
	 * @param height Height of each layer (e.g., 32)
	 * @param maxLayers Maximum number of layers to allocate
	 * @return true if successful
	 */
	bool initialize(int width, int height, int maxLayers);

	/**
	 * Upload RGBA pixel data to a specific layer.
	 * @param layer Layer index (0 to maxLayers-1)
	 * @param rgbaData RGBA pixel data (width * height * 4 bytes)
	 * @return true if successful
	 */
	bool uploadLayer(int layer, const uint8_t* rgbaData);

	/**
	 * Allocate the next available layer.
	 * @return Layer index, or -1 if full
	 */
	int allocateLayer();

	/**
	 * Bind the texture array to a texture unit.
	 * @param unit Texture unit (0, 1, 2, ...)
	 */
	void bind(int unit = 0) const;

	/**
	 * Get the OpenGL texture ID.
	 */
	GLuint getTextureId() const {
		return textureId_;
	}

	/**
	 * Get dimensions.
	 */
	int getWidth() const {
		return width_;
	}
	int getHeight() const {
		return height_;
	}
	int getMaxLayers() const {
		return maxLayers_;
	}
	int getAllocatedLayers() const {
		return allocatedLayers_;
	}

	/**
	 * Check if initialized.
	 */
	bool isInitialized() const {
		return initialized_;
	}

	/**
	 * Release GPU resources.
	 */
	void cleanup();

private:
	GLuint textureId_ = 0;
	int width_ = 0;
	int height_ = 0;
	int maxLayers_ = 0;
	int allocatedLayers_ = 0;
	bool initialized_ = false;
};

#endif
