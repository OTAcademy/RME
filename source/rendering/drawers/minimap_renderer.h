#ifndef RME_RENDERING_DRAWERS_MINIMAP_RENDERER_H_
#define RME_RENDERING_DRAWERS_MINIMAP_RENDERER_H_

#include "app/main.h"
#include "rendering/core/pixel_buffer_object.h"
#include "rendering/core/shader_program.h"
#include "rendering/core/gl_resources.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

class Map;

class MinimapRenderer {
public:
	MinimapRenderer();
	~MinimapRenderer();

	// Non-copyable
	MinimapRenderer(const MinimapRenderer&) = delete;
	MinimapRenderer& operator=(const MinimapRenderer&) = delete;

	/**
	 * Initialize rendering resources (textures, shaders, PBOs).
	 */
	bool initialize();

	/**
	 * Resize the internal texture to match map dimensions.
	 * Warning: This clears the texture logic.
	 */
	void resize(int width, int height);

	/**
	 * Update a region of the minimap texture.
	 * @param map Reference to the map to read tiles from
	 * @param floor Current Z floor
	 * @param x Map X start
	 * @param y Map Y start
	 * @param w Width to update
	 * @param h Height to update
	 */
	void updateRegion(const Map& map, int floor, int x, int y, int w, int h);

	/**
	 * Render the minimap quad.
	 * @param projection Projection matrix (ortho)
	 * @param x Screen X
	 * @param y Screen Y
	 * @param w Screen Width
	 * @param h Screen Height
	 * @param map_x Map X of top-left corner
	 * @param map_y Map Y of top-left corner
	 * @param map_w View width in map tiles
	 * @param map_h View height in map tiles
	 */
	void render(const glm::mat4& projection, int x, int y, int w, int h, float map_x, float map_y, float map_w, float map_h);

	int getWidth() const {
		return width_;
	}
	int getHeight() const {
		return height_;
	}

private:
	void createPaletteTexture();

	std::unique_ptr<GLTextureResource> texture_id_;
	std::unique_ptr<GLTextureResource> palette_texture_id_;
	std::unique_ptr<GLVertexArray> vao_;
	std::unique_ptr<GLBuffer> vbo_;

	int width_ = 0;
	int height_ = 0;

	std::unique_ptr<ShaderProgram> shader_;
	std::unique_ptr<PixelBufferObject> pbo_;

	// Temporary staging buffer for PBO uploads
	// Temporary staging buffer for PBO uploads
	std::vector<uint8_t> stage_buffer_;

	// Tiled configuration
	static constexpr int TILE_SIZE = 2048;
	int rows_ = 0;
	int cols_ = 0;
};

#endif
