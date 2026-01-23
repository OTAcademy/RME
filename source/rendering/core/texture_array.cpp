#include "rendering/core/texture_array.h"
#include <iostream>

TextureArray::TextureArray() {
}

TextureArray::~TextureArray() {
	cleanup();
}

bool TextureArray::initialize(int width, int height, int maxLayers) {
	if (initialized_) {
		std::cerr << "TextureArray: Already initialized" << std::endl;
		return true;
	}

	width_ = width;
	height_ = height;
	maxLayers_ = maxLayers;

	// Create texture array
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureId_);
	if (textureId_ == 0) {
		std::cerr << "TextureArray: Failed to create texture" << std::endl;
		return false;
	}

	// Allocate immutable storage for all layers
	// Using RGBA8 format, no mipmaps (level 1)
	glTextureStorage3D(textureId_, 1, GL_RGBA8, width_, height_, maxLayers_);

	// Set texture parameters
	glTextureParameteri(textureId_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(textureId_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(textureId_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(textureId_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	allocatedLayers_ = 0;
	initialized_ = true;

	std::cout << "TextureArray: Initialized " << width_ << "x" << height_
			  << " with " << maxLayers_ << " layers" << std::endl;

	return true;
}

bool TextureArray::uploadLayer(int layer, const uint8_t* rgbaData) {
	if (!initialized_) {
		std::cerr << "TextureArray: Not initialized" << std::endl;
		return false;
	}
	if (layer < 0 || layer >= maxLayers_) {
		std::cerr << "TextureArray: Layer " << layer << " out of range (max: " << maxLayers_ << ")" << std::endl;
		return false;
	}
	if (rgbaData == nullptr) {
		std::cerr << "TextureArray: Null data for layer " << layer << std::endl;
		return false;
	}

	// Upload to specific layer (z offset = layer, depth = 1 layer)
	glTextureSubImage3D(textureId_, 0, 0, 0, layer, width_, height_, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgbaData);

	return true;
}

int TextureArray::allocateLayer() {
	if (!initialized_) {
		return -1;
	}
	if (allocatedLayers_ >= maxLayers_) {
		std::cerr << "TextureArray: No more layers available (allocated: " << allocatedLayers_ << ")" << std::endl;
		return -1;
	}
	return allocatedLayers_++;
}

void TextureArray::bind(int unit) const {
	if (initialized_) {
		glBindTextureUnit(unit, textureId_);
	}
}

void TextureArray::cleanup() {
	if (textureId_) {
		glDeleteTextures(1, &textureId_);
		textureId_ = 0;
	}
	initialized_ = false;
	allocatedLayers_ = 0;
}
