#include "app/main.h"
#include "rendering/core/text_renderer.h"

// GLAD must be included before NanoVG
#include <glad/glad.h>

#define NANOVG_GL3_IMPLEMENTATION
#include <nanovg.h>
#include <nanovg_gl.h>

#include <iostream>
#include <vector>
#include <fstream>

// Static buffer to hold font data in memory
// Must persist as long as any NanoVG context uses it (lifetime of app essentially)
static std::vector<uint8_t> font_data;

void TextRenderer::LoadFont(NVGcontext* vg) {
	if (!vg) return;

	// Check if font is already loaded in this context
	if (nvgFindFont(vg, "sans") >= 0) {
		return;
	}

	if (font_data.empty()) {
		// Try to load font
		std::vector<std::string> fontPaths = {
			"C:\\Windows\\Fonts\\arial.ttf",
			"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
			"/usr/share/fonts/TTF/DejaVuSans.ttf",
			"/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
			"/usr/share/fonts/liberation/LiberationSans-Regular.ttf"
		};

		for (const auto& path : fontPaths) {
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (file.is_open()) {
				std::streamsize size = file.tellg();
				file.seekg(0, std::ios::beg);

				font_data.resize(size);
				if (file.read((char*)font_data.data(), size)) {
					// std::cout << "Loaded font from: " << path << std::endl;
					break;
				} else {
					font_data.clear();
				}
			}
		}

		if (font_data.empty()) {
			std::cerr << "TextRenderer: Failed to load any font. Text rendering will fail." << std::endl;
			return;
		}
	}

	// nvgCreateFontMem does NOT copy the data, so font_data must persist
	int font = nvgCreateFontMem(vg, "sans", font_data.data(), static_cast<int>(font_data.size()), 0);
	if (font == -1) {
		std::cerr << "TextRenderer: Could not create font 'sans' from memory." << std::endl;
	}
}

void TextRenderer::BeginFrame(NVGcontext* vg, int width, int height) {
	if (!vg) {
		return;
	}
	nvgBeginFrame(vg, width, height, 1.0f); // Assuming 1.0 pixel ratio for now
}

void TextRenderer::EndFrame(NVGcontext* vg) {
	if (!vg) {
		return;
	}
	nvgEndFrame(vg);
}

void TextRenderer::DrawText(NVGcontext* vg, int x, int y, const std::string& text, const glm::vec4& color, float fontSize) {
	if (!vg) return;

	nvgFontSize(vg, fontSize);
	nvgFontFace(vg, "sans");
	nvgFillColor(vg, nvgRGBAf(color.r, color.g, color.b, color.a));
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgText(vg, x, y, text.c_str(), nullptr);
}

void TextRenderer::DrawTextBox(NVGcontext* vg, int x, int y, int width, const std::string& text, const glm::vec4& color, float fontSize) {
	if (!vg) return;

	nvgFontSize(vg, fontSize);
	nvgFontFace(vg, "sans");
	nvgFillColor(vg, nvgRGBAf(color.r, color.g, color.b, color.a));
	nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);
	nvgTextBox(vg, x, y, width, text.c_str(), nullptr);
}

void TextRenderer::DrawRect(NVGcontext* vg, int x, int y, int w, int h, const glm::vec4& color) {
	if (!vg) {
		return;
	}

	nvgBeginPath(vg);
	nvgRect(vg, x, y, w, h);
	nvgFillColor(vg, nvgRGBAf(color.r, color.g, color.b, color.a));
	nvgFill(vg);
}
