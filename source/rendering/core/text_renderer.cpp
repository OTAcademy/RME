#include "main.h"
#include "rendering/core/text_renderer.h"

// GLAD must be included before NanoVG
#include <glad/glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include <iostream>

NVGcontext* TextRenderer::nvgContext = nullptr;
int TextRenderer::fontNormal = -1;

void TextRenderer::Init() {
	if (nvgContext) {
		return;
	}

	// Create NanoVG GL3 context (compatible with GL 4.5 Core if shaders version set correctly,
	// but standard nvgGL3 uses #version 150 which is fine)
	nvgContext = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
	if (!nvgContext) {
		std::cerr << "Failed to init NanoVG." << std::endl;
		return;
	}

	// Load Font - fallback to Windows Arial
	// In a real app, you should bundle a font in a resource/data folder.
	fontNormal = nvgCreateFont(nvgContext, "sans", "C:\\Windows\\Fonts\\arial.ttf");
	if (fontNormal == -1) {
		std::cerr << "Could not add font 'sans' from C:\\Windows\\Fonts\\arial.ttf" << std::endl;
	}
}

void TextRenderer::Shutdown() {
	if (nvgContext) {
		nvgDeleteGL3(nvgContext);
		nvgContext = nullptr;
	}
}

void TextRenderer::BeginFrame(int width, int height) {
	if (!nvgContext) {
		return;
	}
	nvgBeginFrame(nvgContext, width, height, 1.0f); // Assuming 1.0 pixel ratio for now
}

void TextRenderer::EndFrame() {
	if (!nvgContext) {
		return;
	}
	nvgEndFrame(nvgContext);
}

void TextRenderer::DrawText(int x, int y, const std::string& text, const glm::vec4& color, float fontSize) {
	if (!nvgContext || fontNormal == -1) {
		return;
	}

	nvgFontSize(nvgContext, fontSize);
	nvgFontFace(nvgContext, "sans");
	nvgFillColor(nvgContext, nvgRGBAf(color.r, color.g, color.b, color.a));
	nvgTextAlign(nvgContext, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgText(nvgContext, x, y, text.c_str(), nullptr);
}

void TextRenderer::DrawTextBox(int x, int y, int width, const std::string& text, const glm::vec4& color, float fontSize) {
	if (!nvgContext || fontNormal == -1) {
		return;
	}

	nvgFontSize(nvgContext, fontSize);
	nvgFontFace(nvgContext, "sans");
	nvgFillColor(nvgContext, nvgRGBAf(color.r, color.g, color.b, color.a));
	nvgTextAlign(nvgContext, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgTextBox(nvgContext, x, y, width, text.c_str(), nullptr);
}

void TextRenderer::DrawRect(int x, int y, int w, int h, const glm::vec4& color) {
	if (!nvgContext) {
		return;
	}

	nvgBeginPath(nvgContext);
	nvgRect(nvgContext, x, y, w, h);
	nvgFillColor(nvgContext, nvgRGBAf(color.r, color.g, color.b, color.a));
	nvgFill(nvgContext);
}
