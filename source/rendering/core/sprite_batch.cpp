#include "rendering/core/sprite_batch.h"
#include <iostream>
#include <cstring>
#include <utility>
#include <spdlog/spdlog.h>

const char* sprite_batch_vert = R"(
#version 450 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec4 aRect;
layout (location = 3) in vec4 aUV;
layout (location = 4) in vec4 aTint;
layout (location = 5) in float aLayer;

out vec3 TexCoord;
out vec4 Tint;

uniform mat4 uMVP;

void main() {
    vec2 pos = aRect.xy + aPos * aRect.zw;
    gl_Position = uMVP * vec4(pos, 0.0, 1.0);
    TexCoord = vec3(mix(aUV.xy, aUV.zw, aTexCoord), aLayer);
    Tint = aTint;
}
)";

const char* sprite_batch_frag = R"(
#version 450 core
in vec3 TexCoord;
in vec4 Tint;
out vec4 FragColor;

uniform sampler2DArray uAtlas;
uniform vec4 uGlobalTint;

void main() {
    FragColor = texture(uAtlas, TexCoord) * Tint * uGlobalTint;
}
)";

SpriteBatch::SpriteBatch() {
	pending_sprites_.reserve(MAX_SPRITES_PER_BATCH);
}

SpriteBatch::~SpriteBatch() {
	// RAII cleanup
}

SpriteBatch::SpriteBatch(SpriteBatch&& other) noexcept
	:
	shader_(std::move(other.shader_)),
	vao_(std::move(other.vao_)), quad_vbo_(std::move(other.quad_vbo_)), quad_ebo_(std::move(other.quad_ebo_)), ring_buffer_(std::move(other.ring_buffer_)), mdi_renderer_(std::move(other.mdi_renderer_)), pending_sprites_(std::move(other.pending_sprites_)), projection_(other.projection_), global_tint_(other.global_tint_), in_batch_(other.in_batch_), use_mdi_(other.use_mdi_), draw_call_count_(other.draw_call_count_), sprite_count_(other.sprite_count_) {
	other.in_batch_ = false;
}

SpriteBatch& SpriteBatch::operator=(SpriteBatch&& other) noexcept {
	if (this != &other) {
		shader_ = std::move(other.shader_);
		vao_ = std::move(other.vao_);
		quad_vbo_ = std::move(other.quad_vbo_);
		quad_ebo_ = std::move(other.quad_ebo_);
		ring_buffer_ = std::move(other.ring_buffer_);
		mdi_renderer_ = std::move(other.mdi_renderer_);
		pending_sprites_ = std::move(other.pending_sprites_);
		projection_ = other.projection_;
		global_tint_ = other.global_tint_;
		in_batch_ = other.in_batch_;
		use_mdi_ = other.use_mdi_;
		draw_call_count_ = other.draw_call_count_;
		sprite_count_ = other.sprite_count_;

		other.in_batch_ = false;
	}
	return *this;
}

bool SpriteBatch::initialize() {
	// Load shader
	shader_ = std::make_unique<ShaderProgram>();
	if (!shader_->Load(sprite_batch_vert, sprite_batch_frag)) {
		spdlog::error("SpriteBatch: Failed to load shader");
		return false;
	}

	// Initialize RingBuffer
	if (!ring_buffer_.initialize(sizeof(SpriteInstance), MAX_SPRITES_PER_BATCH)) {
		spdlog::error("SpriteBatch: Failed to initialize RingBuffer");
		return false;
	}

	// Create VAO and static buffers (RAII)
	vao_ = std::make_unique<GLVertexArray>();
	quad_vbo_ = std::make_unique<GLBuffer>();
	quad_ebo_ = std::make_unique<GLBuffer>();

	// Unit quad geometry
	float quad_vertices[] = {
		0.0f, 0.0f, 0.0f, 0.0f, // pos.xy, tex.xy
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f
	};
	unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

	// Upload static geometry
	glNamedBufferStorage(quad_vbo_->GetID(), sizeof(quad_vertices), quad_vertices, 0);
	glNamedBufferStorage(quad_ebo_->GetID(), sizeof(indices), indices, 0);

	// Bind Quad VBO/EBO to VAO (DSA)
	glVertexArrayVertexBuffer(vao_->GetID(), 0, quad_vbo_->GetID(), 0, 4 * sizeof(float));
	glVertexArrayElementBuffer(vao_->GetID(), quad_ebo_->GetID());

	// Loc 0: position (vec2)
	glEnableVertexArrayAttrib(vao_->GetID(), 0);
	glVertexArrayAttribFormat(vao_->GetID(), 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_->GetID(), 0, 0);

	// Loc 1: texcoord (vec2)
	glEnableVertexArrayAttrib(vao_->GetID(), 1);
	glVertexArrayAttribFormat(vao_->GetID(), 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
	glVertexArrayAttribBinding(vao_->GetID(), 1, 0);

	// Bind RingBuffer for instance data
	// Bind RingBuffer for instance data to binding point 1 (DSA)
	glVertexArrayVertexBuffer(vao_->GetID(), 1, ring_buffer_.getBufferId(), 0, sizeof(SpriteInstance));
	glVertexArrayBindingDivisor(vao_->GetID(), 1, 1);

	// Loc 2: rect (vec4) - instance
	glEnableVertexArrayAttrib(vao_->GetID(), 2);
	glVertexArrayAttribFormat(vao_->GetID(), 2, 4, GL_FLOAT, GL_FALSE, offsetof(SpriteInstance, x));
	glVertexArrayAttribBinding(vao_->GetID(), 2, 1);

	// Loc 3: uv (vec4) - instance
	glEnableVertexArrayAttrib(vao_->GetID(), 3);
	glVertexArrayAttribFormat(vao_->GetID(), 3, 4, GL_FLOAT, GL_FALSE, offsetof(SpriteInstance, u_min));
	glVertexArrayAttribBinding(vao_->GetID(), 3, 1);

	// Loc 4: tint (vec4) - instance
	glEnableVertexArrayAttrib(vao_->GetID(), 4);
	glVertexArrayAttribFormat(vao_->GetID(), 4, 4, GL_FLOAT, GL_FALSE, offsetof(SpriteInstance, r));
	glVertexArrayAttribBinding(vao_->GetID(), 4, 1);

	// Loc 5: layer (float) - instance
	glEnableVertexArrayAttrib(vao_->GetID(), 5);
	glVertexArrayAttribFormat(vao_->GetID(), 5, 1, GL_FLOAT, GL_FALSE, offsetof(SpriteInstance, atlas_layer));
	glVertexArrayAttribBinding(vao_->GetID(), 5, 1);

	// Initialize MDI
	if (mdi_renderer_.initialize()) {
		use_mdi_ = true;
		spdlog::info("SpriteBatch: MDI renderer initialized");
	} else {
		spdlog::warn("SpriteBatch: MDI renderer failed to initialize, using fallback path");
	}

	spdlog::info("SpriteBatch initialized successfully (VAO: {})", vao_->GetID());
	return true;
}

void SpriteBatch::begin(const glm::mat4& projection) {
	projection_ = projection;
	pending_sprites_.clear();
	in_batch_ = true;
	draw_call_count_ = 0;
	sprite_count_ = 0;
	last_bound_vao_ = 0;
	global_tint_ = glm::vec4(1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader_->Use();
	shader_->SetMat4("uMVP", projection_);
	shader_->SetInt("uAtlas", 0);
	shader_->SetVec4("uGlobalTint", global_tint_);
}

void SpriteBatch::setGlobalTint(float r, float g, float b, float a) {
	if (!in_batch_) {
		return;
	}

	// If pending sprites exist, must flush to apply previous tint
	if (!pending_sprites_.empty()) {
		// Warning: This causes a flush and state change
		// We can't access AtlasManager here, so we must assume calling code
		// flushes or sets tint at appropriate times.
		// Ideally setGlobalTint is called when batch is empty.
	}

	global_tint_ = glm::vec4(r, g, b, a);
	shader_->SetVec4("uGlobalTint", global_tint_);
}

void SpriteBatch::ensureCapacity(size_t capacity) {
	if (pending_sprites_.capacity() < capacity) {
		pending_sprites_.reserve(capacity);
	}
}

void SpriteBatch::draw(float x, float y, float w, float h, const AtlasRegion& region) {
	draw(x, y, w, h, region, 1.0f, 1.0f, 1.0f, 1.0f);
}

void SpriteBatch::draw(float x, float y, float w, float h, const AtlasRegion& region, float r, float g, float b, float a) {
	if (!in_batch_) {
		return;
	}

	SpriteInstance& inst = pending_sprites_.emplace_back();
	inst.x = x;
	inst.y = y;
	inst.w = w;
	inst.h = h;
	inst.u_min = region.u_min;
	inst.v_min = region.v_min;
	inst.u_max = region.u_max;
	inst.v_max = region.v_max;
	inst.r = r;
	inst.g = g;
	inst.b = b;
	inst.a = a;
	inst.atlas_layer = static_cast<float>(region.atlas_index);
	// Pad initialized to 0 implicitly or structurally? C++ struct fields not init
	inst._pad1 = inst._pad2 = inst._pad3 = 0.0f;
}

void SpriteBatch::drawRect(float x, float y, float w, float h, const glm::vec4& color, const AtlasManager& atlas_manager) {
	const AtlasRegion* region = atlas_manager.getWhitePixel();
	if (region) {
		draw(x, y, w, h, *region, color.r, color.g, color.b, color.a);
	}
}

void SpriteBatch::drawRectLines(float x, float y, float w, float h, const glm::vec4& color, const AtlasManager& atlas_manager) {
	// Top
	drawRect(x, y, w, 1.0f, color, atlas_manager);
	// Bottom
	drawRect(x, y + h - 1.0f, w, 1.0f, color, atlas_manager);
	// Left
	drawRect(x, y + 1.0f, 1.0f, h - 2.0f, color, atlas_manager);
	// Right
	drawRect(x + w - 1.0f, y + 1.0f, 1.0f, h - 2.0f, color, atlas_manager);
}

void SpriteBatch::flush(const AtlasManager& atlas_manager) {
	if (pending_sprites_.empty()) {
		return;
	}

	atlas_manager.bind(0);

	if (last_bound_vao_ != vao_->GetID()) {
		glBindVertexArray(vao_->GetID());
		last_bound_vao_ = vao_->GetID();
	}

	glBindBuffer(GL_ARRAY_BUFFER, ring_buffer_.getBufferId());

	if (use_mdi_) {
		// MDI Path: Aggregate all batches into one draw call
		// If we fill the entire ring buffer (3 sections), we must flush intermediate draws
		// to avoid overwriting the beginning of the buffer before it is executed.

		mdi_renderer_.clear();

		// DSA handles attribute binding, so we don't need to touch glVertexAttribPointer here.
		// baseInstance in the Indirect command handles the offset into the buffer.

		std::vector<size_t> used_sections;
		size_t total = pending_sprites_.size();
		size_t processed = 0;
		size_t max_batch = ring_buffer_.getCapacity();

		while (processed < total) {
			// CHECK FOR OVERFLOW: If we have used all sections, we MUST flush now!
			if (used_sections.size() >= RingBuffer::BUFFER_COUNT) {
				mdi_renderer_.upload();
				mdi_renderer_.execute();
				draw_call_count_++;

				for (size_t section_idx : used_sections) {
					ring_buffer_.fence(section_idx);
				}

				mdi_renderer_.clear();
				used_sections.clear();
			}

			size_t batch_size = std::min(total - processed, max_batch);

			// Wait for current section to be free
			// (If we just flushed above, the fence was placed, and waitAndMap will block correctly if GPU is slow)
			void* ptr = ring_buffer_.waitAndMap(batch_size);
			if (!ptr) {
				break;
			}

			// Copy instance data
			memcpy(ptr, pending_sprites_.data() + processed, batch_size * sizeof(SpriteInstance));
			ring_buffer_.finishWrite();

			// Calculate base instance for MDI
			size_t offset = ring_buffer_.getCurrentSectionOffset();
			GLuint baseInstance = static_cast<GLuint>(offset / sizeof(SpriteInstance));

			// Add command
			mdi_renderer_.addDrawCommand(6, static_cast<GLuint>(batch_size), 0, 0, baseInstance);

			// Track used section and advance
			used_sections.push_back(ring_buffer_.getCurrentSectionIndex());
			ring_buffer_.advance();

			processed += batch_size;
			sprite_count_ += (int)batch_size;
		}

		// Flush remaining commands
		if (!used_sections.empty()) {
			mdi_renderer_.upload();
			mdi_renderer_.execute();
			draw_call_count_++;

			for (size_t section_idx : used_sections) {
				ring_buffer_.fence(section_idx);
			}
		}

	} else {
		// Fallback Path: Standard Instanced Rendering Looping
		size_t total = pending_sprites_.size();
		size_t processed = 0;
		size_t max_batch = ring_buffer_.getMaxElements();

		while (processed < total) {
			size_t batch_size = std::min(total - processed, max_batch);
			void* ptr = ring_buffer_.waitAndMap(batch_size);
			if (!ptr) {
				break;
			}

			memcpy(ptr, pending_sprites_.data() + processed, batch_size * sizeof(SpriteInstance));
			ring_buffer_.finishWrite();

			size_t offset = ring_buffer_.getCurrentSectionOffset();

			// Fallback: update VAO binding offset (DSA)
			// Binding point 1 is used for instance data (Attributes 2, 3, 4, 5)
			glVertexArrayVertexBuffer(vao_->GetID(), 1, ring_buffer_.getBufferId(), offset, sizeof(SpriteInstance));

			glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, (GLsizei)batch_size);

			// Standard signal: Fence current and advance
			ring_buffer_.signalFinished();

			processed += batch_size;
			draw_call_count_++;
			sprite_count_ += (int)batch_size;
		}
	}

	pending_sprites_.clear();
}

void SpriteBatch::end(const AtlasManager& atlas_manager) {
	if (!in_batch_) {
		return;
	}

	flush(atlas_manager);

	in_batch_ = false;
	glBindVertexArray(0);
	glDisable(GL_BLEND);
}
