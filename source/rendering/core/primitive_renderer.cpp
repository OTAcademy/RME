#include "rendering/core/primitive_renderer.h"
#include <iostream>
#include <spdlog/spdlog.h>

const char* primitive_vert = R"(
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;

out vec4 Color;
uniform mat4 uMVP;

void main() {
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
    Color = aColor;
}
)";

const char* primitive_frag = R"(
#version 450 core
in vec4 Color;
out vec4 FragColor;

void main() {
    FragColor = Color;
}
)";

PrimitiveRenderer::PrimitiveRenderer() {
	triangle_verts_.reserve(MAX_VERTICES);
	line_verts_.reserve(MAX_VERTICES);
}

PrimitiveRenderer::~PrimitiveRenderer() {
	shutdown();
}

void PrimitiveRenderer::initialize() {
	shader_ = std::make_unique<ShaderProgram>();
	if (!shader_->Load(primitive_vert, primitive_frag)) {
		spdlog::error("PrimitiveRenderer: Shader load failed");
	}

	vao_ = std::make_unique<GLVertexArray>();
	vbo_ = std::make_unique<GLBuffer>();

	// Allocate buffer
	glNamedBufferStorage(vbo_->GetID(), MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_STORAGE_BIT);

	// DSA setup
	glVertexArrayVertexBuffer(vao_->GetID(), 0, vbo_->GetID(), 0, sizeof(Vertex));

	// Pos
	glEnableVertexArrayAttrib(vao_->GetID(), 0);
	glVertexArrayAttribFormat(vao_->GetID(), 0, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribBinding(vao_->GetID(), 0, 0);

	// Color
	glEnableVertexArrayAttrib(vao_->GetID(), 1);
	glVertexArrayAttribFormat(vao_->GetID(), 1, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
	glVertexArrayAttribBinding(vao_->GetID(), 1, 0);

	spdlog::info("PrimitiveRenderer initialized (VAO: {}, VBO: {})", vao_->GetID(), vbo_->GetID());
}

void PrimitiveRenderer::shutdown() {
	vao_.reset();
	vbo_.reset();
}

void PrimitiveRenderer::setProjectionMatrix(const glm::mat4& projection) {
	projection_ = projection;
}

void PrimitiveRenderer::drawTriangle(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3, const glm::vec4& color) {
	if (triangle_verts_.size() + 3 > MAX_VERTICES) {
		flushTriangles();
	}
	triangle_verts_.push_back({ p1, color });
	triangle_verts_.push_back({ p2, color });
	triangle_verts_.push_back({ p3, color });
}

void PrimitiveRenderer::drawLine(const glm::vec2& p1, const glm::vec2& p2, const glm::vec4& color) {
	if (line_verts_.size() + 2 > MAX_VERTICES) {
		flushLines();
	}
	line_verts_.push_back({ p1, color });
	line_verts_.push_back({ p2, color });
}

void PrimitiveRenderer::drawRect(const glm::vec4& rect, const glm::vec4& color) {
	// rect: x, y, w, h
	float x = rect.x;
	float y = rect.y;
	float w = rect.z;
	float h = rect.w;

	glm::vec2 p1(x, y);
	glm::vec2 p2(x + w, y);
	glm::vec2 p3(x + w, y + h);
	glm::vec2 p4(x, y + h);

	// Triangle 1: p1, p2, p3
	drawTriangle(p1, p2, p3, color);
	// Triangle 2: p3, p4, p1
	drawTriangle(p3, p4, p1, color);
}

void PrimitiveRenderer::drawBox(const glm::vec4& rect, const glm::vec4& color, float thickness) {
	float x = rect.x;
	float y = rect.y;
	float w = rect.z;
	float h = rect.w;
	float t = thickness;

	// Top
	drawRect(glm::vec4(x, y, w, t), color);
	// Bottom
	drawRect(glm::vec4(x, y + h - t, w, t), color);
	// Left (excluding corners to avoid overdraw if alpha < 1, though standard drawRect handles blending)
	// Actually, if we use standard blending, drawing corners twice makes them darker.
	// So we should strictly non-overlap.
	// Top and Bottom are full width.
	// Left and Right should be between Top and Bottom.

	// Left
	drawRect(glm::vec4(x, y + t, t, h - 2 * t), color);
	// Right
	drawRect(glm::vec4(x + w - t, y + t, t, h - 2 * t), color);
}

void PrimitiveRenderer::flush() {
	flushTriangles();
	flushLines();
}

void PrimitiveRenderer::flushTriangles() {
	if (triangle_verts_.empty()) {
		return;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader_->Use();
	shader_->SetMat4("uMVP", projection_);

	glNamedBufferSubData(vbo_->GetID(), 0, triangle_verts_.size() * sizeof(Vertex), triangle_verts_.data());

	glBindVertexArray(vao_->GetID());
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)triangle_verts_.size());
	glBindVertexArray(0);

	glDisable(GL_BLEND);

	triangle_verts_.clear();
}

void PrimitiveRenderer::flushLines() {
	if (line_verts_.empty()) {
		return;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader_->Use();
	shader_->SetMat4("uMVP", projection_);

	glNamedBufferSubData(vbo_->GetID(), 0, line_verts_.size() * sizeof(Vertex), line_verts_.data());

	glBindVertexArray(vao_->GetID());
	glDrawArrays(GL_LINES, 0, (GLsizei)line_verts_.size());
	glBindVertexArray(0);

	glDisable(GL_BLEND);

	line_verts_.clear();
}
