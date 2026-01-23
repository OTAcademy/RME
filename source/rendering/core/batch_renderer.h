#ifndef RME_RENDERING_CORE_BATCH_RENDERER_H_
#define RME_RENDERING_CORE_BATCH_RENDERER_H_

#include "main.h"
#include <vector>
#include "shader_program.h"

struct Vertex {
	glm::vec2 position;
	glm::vec2 texCoord;
	glm::vec4 color; // Normalized 0-1
};

class BatchRenderer {
public:
	static void Init();
	static void Shutdown();

	static void Begin();
	static void End();
	static void Flush();

	static void SetMatrices(const glm::mat4& projection, const glm::mat4& view);

	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	static void DrawTextureQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, GLuint textureID);
	static void DrawTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec4& color);
	static void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color);

	// Immediate mode emulation
	static void SetColor(const glm::vec4& color);
	static void SetTexCoord(const glm::vec2& texCoord);
	static void SubmitVertex(const glm::vec2& position);

	static ShaderProgram& GetShader() {
		return *shader;
	}

private:
	static void InitRenderData();

	static const size_t MAX_QUADS = 10000;
	static const size_t MAX_VERTICES = MAX_QUADS * 4;
	static const size_t MAX_INDICES = MAX_QUADS * 6;

	static GLuint VAO;
	static GLuint VBO;
	static GLuint EBO;

	static std::vector<Vertex> vertices;
	static std::vector<uint32_t> indices;
	static size_t indexCount;

	static GLuint whiteTextureID;
	static GLuint currentTextureID;
	static GLenum currentPrimitiveMode;

	static ShaderProgram* shader;

	// Immediate mode state
	static glm::vec4 currentColor;
	static glm::vec2 currentTexCoord;

	static int refCount;
};

#endif
