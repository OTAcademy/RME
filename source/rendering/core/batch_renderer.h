#ifndef RME_RENDERING_CORE_BATCH_RENDERER_H_
#define RME_RENDERING_CORE_BATCH_RENDERER_H_

#include "main.h"
#include <vector>
#include "main.h"
#include <vector>
#include <memory>
#include "shader_program.h"

// Forward declarations
class AtlasManager;
struct AtlasRegion;

// Vertex with 3D texture coordinates for texture array sampling
struct Vertex {
	glm::vec2 position;
	glm::vec3 texCoord; // u, v, layer
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

	// Atlas-based rendering (Phase 2) - uses AtlasRegion with UV coordinates
	static void SetAtlasManager(AtlasManager* atlas);
	static void DrawAtlasQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const AtlasRegion* region);

	// Legacy single-texture rendering (for backward compatibility)
	static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
	// External texture rendering (e.g. LightMap)
	static void DrawExternalTexture(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, GLuint textureID);
	static void DrawTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec4& color);
	static void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color);

	// Immediate mode emulation
	static void SetColor(const glm::vec4& color);
	static void SetTexCoord(const glm::vec2& texCoord);
	static void SubmitVertex(const glm::vec2& position);

	static ShaderProgram& GetShader() {
		return *externalShader;
	}

private:
	static void InitRenderData();
	static void FlushAtlasBatch(); // Flush for atlas mode
	static void FlushExternalBatch(); // Flush for external texture mode

	static const size_t MAX_QUADS = 50000;
	static const size_t MAX_VERTICES = MAX_QUADS * 4;
	static const size_t MAX_INDICES = MAX_QUADS * 6;

	static GLuint VAO;
	static GLuint VBO;
	static GLuint EBO;

	static std::vector<Vertex> vertices;
	static std::vector<uint32_t> indices;

	static GLuint currentTextureID;
	static GLenum currentPrimitiveMode;

	static std::unique_ptr<ShaderProgram> externalShader; // Generic 2D shader
	static std::unique_ptr<ShaderProgram> atlasShader; // Texture array shader

	static AtlasManager* currentAtlas;
	static const AtlasRegion* whiteRegion;
	static bool usingAtlas; // true = atlas, false = external

	// Immediate mode state
	static glm::vec4 currentColor;
	static glm::vec2 currentTexCoord;

	static int refCount;
};

#endif
