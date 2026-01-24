#ifndef RME_RENDERING_CORE_PRIMITIVE_RENDERER_H_
#define RME_RENDERING_CORE_PRIMITIVE_RENDERER_H_

#include "app/main.h"
#include "rendering/core/shader_program.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

/**
 * Batched renderer for 2D primitives (lines, triangles).
 * Used for UI elements, selection boxes, and debug geometry.
 */
class PrimitiveRenderer {
public:
	PrimitiveRenderer();
	~PrimitiveRenderer();

	// Singleton access or instance? MapDrawer will own it.
	// But static access was convenient. We will pass it around.

	void initialize();
	void shutdown();

	void setProjectionMatrix(const glm::mat4& projection);

	void drawTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec4& color);
	void drawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color);

	void flush();

private:
	struct Vertex {
		glm::vec2 pos;
		glm::vec4 color;
	};

	void flushTriangles();
	void flushLines();

	std::unique_ptr<ShaderProgram> shader_;
	GLuint vao_ = 0;
	GLuint vbo_ = 0;

	std::vector<Vertex> triangle_verts_;
	std::vector<Vertex> line_verts_;

	glm::mat4 projection_ { 1.0f };

	static constexpr size_t MAX_VERTICES = 10000;
};

#endif
