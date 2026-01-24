#include "rendering/core/primitive_renderer.h"
#include <iostream>

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
		std::cerr << "PrimitiveRenderer: Shader load failed" << std::endl;
	}

	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);

	// Allocate buffer
	glNamedBufferStorage(vbo_, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_STORAGE_BIT);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	// Pos
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Color
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec2)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void PrimitiveRenderer::shutdown() {
	if (vao_) {
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;
	}
	if (vbo_) {
		glDeleteBuffers(1, &vbo_);
		vbo_ = 0;
	}
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

void PrimitiveRenderer::flush() {
	flushTriangles();
	flushLines();
}

void PrimitiveRenderer::flushTriangles() {
	if (triangle_verts_.empty()) {
		return;
	}

	shader_->Use();
	shader_->SetMat4("uMVP", projection_);

	glNamedBufferSubData(vbo_, 0, triangle_verts_.size() * sizeof(Vertex), triangle_verts_.data());

	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)triangle_verts_.size());
	glBindVertexArray(0);

	triangle_verts_.clear();
}

void PrimitiveRenderer::flushLines() {
	if (line_verts_.empty()) {
		return;
	}

	shader_->Use();
	shader_->SetMat4("uMVP", projection_);

	glNamedBufferSubData(vbo_, 0, line_verts_.size() * sizeof(Vertex), line_verts_.data());

	glBindVertexArray(vao_);
	glDrawArrays(GL_LINES, 0, (GLsizei)line_verts_.size());
	glBindVertexArray(0);

	line_verts_.clear();
}
