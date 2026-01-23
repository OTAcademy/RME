#include "main.h"
#include "rendering/core/batch_renderer.h"
#include "rendering/core/atlas_manager.h"
#include <iostream>
#include <spdlog/spdlog.h>

// Static members
int BatchRenderer::refCount = 0;
GLuint BatchRenderer::VAO = 0;
GLuint BatchRenderer::VBO = 0;
GLuint BatchRenderer::EBO = 0;
std::vector<Vertex> BatchRenderer::vertices;
std::vector<uint32_t> BatchRenderer::indices;
GLuint BatchRenderer::whiteTextureID = 0;
GLuint BatchRenderer::currentTextureID = 0;
GLenum BatchRenderer::currentPrimitiveMode = GL_TRIANGLES;
ShaderProgram* BatchRenderer::shader = nullptr;
ShaderProgram* BatchRenderer::atlasShader = nullptr;
AtlasManager* BatchRenderer::currentAtlas = nullptr;
bool BatchRenderer::usingAtlas = false;

glm::vec4 BatchRenderer::currentColor = glm::vec4(1.0f);
glm::vec2 BatchRenderer::currentTexCoord = glm::vec2(0.0f);

static glm::mat4 s_ProjectionMatrix = glm::mat4(1.0f);
static glm::mat4 s_ViewMatrix = glm::mat4(1.0f);

void BatchRenderer::SetMatrices(const glm::mat4& projection, const glm::mat4& view) {
	s_ProjectionMatrix = projection;
	s_ViewMatrix = view;
	if (shader) {
		shader->Use();
		shader->SetMat4("projection", s_ProjectionMatrix);
		shader->SetMat4("view", s_ViewMatrix);
	}
	if (atlasShader) {
		atlasShader->Use();
		atlasShader->SetMat4("projection", s_ProjectionMatrix);
		atlasShader->SetMat4("view", s_ViewMatrix);
	}
}

// Legacy single-texture shader
const char* vertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec2 TexCoord;
out vec4 Color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord.xy;
    Color = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 450 core
in vec2 TexCoord;
in vec4 Color;

out vec4 FragColor;

uniform sampler2D image;

void main() {
    FragColor = texture(image, TexCoord) * Color;
}
)";

// Texture Array shader (Phase 2) - uses 3D texture coordinates
const char* atlasVertexShaderSource = R"(
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aTexCoord;
layout (location = 2) in vec4 aColor;

out vec3 TexCoord;
out vec4 Color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    gl_Position = projection * view * vec4(aPos, 0.0, 1.0);
    TexCoord = aTexCoord;
    Color = aColor;
}
)";

const char* atlasFragmentShaderSource = R"(
#version 450 core
in vec3 TexCoord;
in vec4 Color;

out vec4 FragColor;

uniform sampler2DArray atlas;

void main() {
    FragColor = texture(atlas, TexCoord) * Color;
}
)";

void BatchRenderer::Init() {
	if (refCount++ > 0) {
		spdlog::info("BatchRenderer::Init skipped (refCount={})", refCount);
		return;
	}
	spdlog::info("BatchRenderer::Init starting");
	InitRenderData();

	// Load legacy shader
	shader = newd ShaderProgram();
	if (!shader->Load(vertexShaderSource, fragmentShaderSource)) {
		spdlog::error("Failed to load BatchRenderer legacy shader");
	} else {
		spdlog::info("BatchRenderer legacy shader loaded successfully");
	}

	// Load atlas shader
	atlasShader = newd ShaderProgram();
	if (!atlasShader->Load(atlasVertexShaderSource, atlasFragmentShaderSource)) {
		spdlog::error("Failed to load BatchRenderer atlas shader");
	} else {
		spdlog::info("BatchRenderer atlas shader loaded successfully");
	}

	// Create 1x1 white texture for untextured drawing
	glCreateTextures(GL_TEXTURE_2D, 1, &whiteTextureID);
	glTextureStorage2D(whiteTextureID, 1, GL_RGBA8, 1, 1);
	uint32_t white = 0xFFFFFFFF;
	glTextureSubImage2D(whiteTextureID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &white);
	spdlog::info("BatchRenderer::Init complete - whiteTextureID={}, VAO={}, VBO={}, EBO={}", whiteTextureID, VAO, VBO, EBO);
}

void BatchRenderer::Shutdown() {
	if (--refCount > 0) {
		return;
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteTextures(1, &whiteTextureID);
	delete shader;
	delete atlasShader;
}

void BatchRenderer::InitRenderData() {
	glCreateVertexArrays(1, &VAO);

	glCreateBuffers(1, &VBO);
	glNamedBufferData(VBO, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	glCreateBuffers(1, &EBO);
	glNamedBufferData(EBO, MAX_INDICES * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(VAO, EBO);

	// Pos (vec2)
	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
	glVertexArrayAttribBinding(VAO, 0, 0);

	// TexCoord (vec3 for u, v, layer)
	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
	glVertexArrayAttribBinding(VAO, 1, 0);

	// Color (vec4)
	glEnableVertexArrayAttrib(VAO, 2);
	glVertexArrayAttribFormat(VAO, 2, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
	glVertexArrayAttribBinding(VAO, 2, 0);
}

void BatchRenderer::SetAtlasManager(AtlasManager* atlas) {
	if (currentAtlas != atlas) {
		Flush();
		currentAtlas = atlas;
	}
	usingAtlas = (atlas != nullptr && atlas->isValid());
}

void BatchRenderer::Begin() {
	vertices.clear();
	indices.clear();
	currentTextureID = whiteTextureID;
	currentPrimitiveMode = GL_TRIANGLES;
	usingAtlas = false;
	currentAtlas = nullptr;
}

void BatchRenderer::End() {
	Flush();
}

void BatchRenderer::Flush() {
	if (vertices.empty()) {
		return;
	}

	if (usingAtlas && currentAtlas) {
		FlushAtlasBatch();
	} else {
		FlushLegacyBatch();
	}
}

void BatchRenderer::FlushAtlasBatch() {
	glNamedBufferSubData(VBO, 0, vertices.size() * sizeof(Vertex), vertices.data());

	if (currentPrimitiveMode == GL_TRIANGLES && !indices.empty()) {
		glNamedBufferSubData(EBO, 0, indices.size() * sizeof(uint32_t), indices.data());
	}

	atlasShader->Use();
	atlasShader->SetMat4("projection", s_ProjectionMatrix);
	atlasShader->SetMat4("view", s_ViewMatrix);
	currentAtlas->bind(0);
	atlasShader->SetInt("atlas", 0);

	glBindVertexArray(VAO);

	if (currentPrimitiveMode == GL_TRIANGLES) {
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	} else if (currentPrimitiveMode == GL_LINES) {
		glDrawArrays(GL_LINES, 0, vertices.size());
	}

	vertices.clear();
	indices.clear();
}

void BatchRenderer::FlushLegacyBatch() {
	static int s_flushCount = 0;
	static int s_logEveryN = 1000; // Log every 1000th flush to avoid spam

	glNamedBufferSubData(VBO, 0, vertices.size() * sizeof(Vertex), vertices.data());

	if (currentPrimitiveMode == GL_TRIANGLES && !indices.empty()) {
		glNamedBufferSubData(EBO, 0, indices.size() * sizeof(uint32_t), indices.data());
	}

	shader->Use();
	// IMPORTANT: Set matrices every flush - they must be set after Use()
	shader->SetMat4("projection", s_ProjectionMatrix);
	shader->SetMat4("view", s_ViewMatrix);
	glBindTextureUnit(0, currentTextureID);
	shader->SetInt("image", 0);

	glBindVertexArray(VAO);

	if (currentPrimitiveMode == GL_TRIANGLES) {
		if (s_flushCount % s_logEveryN == 0) {
			spdlog::info("FlushLegacy #{}: textureID={}, vertices={}, indices={}, VAO={}", s_flushCount, currentTextureID, vertices.size(), indices.size(), VAO);
		}
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	} else if (currentPrimitiveMode == GL_LINES) {
		glDrawArrays(GL_LINES, 0, vertices.size());
	}

	s_flushCount++;
	vertices.clear();
	indices.clear();
}

void BatchRenderer::DrawAtlasQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, const AtlasRegion* region) {
	if (!region || !currentAtlas) {
		return;
	}

	// Switch to atlas mode if we were using legacy textures
	if (!usingAtlas && !vertices.empty()) {
		Flush();
	}
	usingAtlas = true;

	if (currentPrimitiveMode != GL_TRIANGLES || vertices.size() + 4 > MAX_VERTICES || indices.size() + 6 > MAX_INDICES) {
		Flush();
		currentPrimitiveMode = GL_TRIANGLES;
	}

	float x = position.x;
	float y = position.y;
	float w = size.x;
	float h = size.y;

	// Use UV coordinates from AtlasRegion
	float u0 = region->u_min;
	float v0 = region->v_min;
	float u1 = region->u_max;
	float v1 = region->v_max;
	float layer = static_cast<float>(region->atlas_index);

	uint32_t offset = vertices.size();

	// Vertices with proper UV coordinates from atlas
	vertices.push_back({ { x, y }, { u0, v0, layer }, color });
	vertices.push_back({ { x + w, y }, { u1, v0, layer }, color });
	vertices.push_back({ { x + w, y + h }, { u1, v1, layer }, color });
	vertices.push_back({ { x, y + h }, { u0, v1, layer }, color });

	indices.push_back(offset + 0);
	indices.push_back(offset + 1);
	indices.push_back(offset + 2);

	indices.push_back(offset + 2);
	indices.push_back(offset + 3);
	indices.push_back(offset + 0);
}

void BatchRenderer::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
	DrawTextureQuad(position, size, color, whiteTextureID);
}

void BatchRenderer::DrawTextureQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color, GLuint textureID) {
	// Switch to legacy mode if we were using atlas
	if (usingAtlas) {
		Flush();
		usingAtlas = false;
	}

	if (currentPrimitiveMode != GL_TRIANGLES || textureID != currentTextureID || vertices.size() + 4 > MAX_VERTICES || indices.size() + 6 > MAX_INDICES) {
		Flush();
		currentTextureID = textureID;
		currentPrimitiveMode = GL_TRIANGLES;
	}

	float x = position.x;
	float y = position.y;
	float w = size.x;
	float h = size.y;

	uint32_t offset = vertices.size();

	// Layer 0 for legacy mode
	vertices.push_back({ { x, y }, { 0.0f, 0.0f, 0.0f }, color });
	vertices.push_back({ { x + w, y }, { 1.0f, 0.0f, 0.0f }, color });
	vertices.push_back({ { x + w, y + h }, { 1.0f, 1.0f, 0.0f }, color });
	vertices.push_back({ { x, y + h }, { 0.0f, 1.0f, 0.0f }, color });

	indices.push_back(offset + 0);
	indices.push_back(offset + 1);
	indices.push_back(offset + 2);

	indices.push_back(offset + 2);
	indices.push_back(offset + 3);
	indices.push_back(offset + 0);
}

void BatchRenderer::DrawTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec4& color) {
	if (usingAtlas) {
		Flush();
		usingAtlas = false;
	}

	if (currentPrimitiveMode != GL_TRIANGLES || vertices.size() + 3 > MAX_VERTICES || indices.size() + 3 > MAX_INDICES) {
		Flush();
		currentTextureID = whiteTextureID;
		currentPrimitiveMode = GL_TRIANGLES;
	}

	uint32_t offset = vertices.size();

	vertices.push_back({ p1, { 0.0f, 0.0f, 0.0f }, color });
	vertices.push_back({ p2, { 0.5f, 0.0f, 0.0f }, color });
	vertices.push_back({ p3, { 1.0f, 1.0f, 0.0f }, color });

	indices.push_back(offset + 0);
	indices.push_back(offset + 1);
	indices.push_back(offset + 2);
}

void BatchRenderer::DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color) {
	if (usingAtlas) {
		Flush();
		usingAtlas = false;
	}

	if (currentPrimitiveMode != GL_LINES || vertices.size() + 2 > MAX_VERTICES) {
		Flush();
		currentTextureID = whiteTextureID;
		currentPrimitiveMode = GL_LINES;
	}

	vertices.push_back({ start, { 0.0f, 0.0f, 0.0f }, color });
	vertices.push_back({ end, { 1.0f, 1.0f, 0.0f }, color });
}

// Immediate mode emulations
void BatchRenderer::SetColor(const glm::vec4& color) {
	currentColor = color;
}

void BatchRenderer::SetTexCoord(const glm::vec2& texCoord) {
	currentTexCoord = texCoord;
}

void BatchRenderer::SubmitVertex(const glm::vec2& position) {
	if (currentPrimitiveMode != GL_TRIANGLES || vertices.size() + 1 > MAX_VERTICES) {
		Flush();
		currentPrimitiveMode = GL_TRIANGLES;
	}

	vertices.push_back({ position, glm::vec3(currentTexCoord, 0.0f), currentColor });

	// GL_QUADS emulation: Every 4 vertices = 2 triangles
	if (vertices.size() % 4 == 0) {
		uint32_t offset = vertices.size() - 4;
		indices.push_back(offset + 0);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);

		indices.push_back(offset + 2);
		indices.push_back(offset + 3);
		indices.push_back(offset + 0);
	}
}
