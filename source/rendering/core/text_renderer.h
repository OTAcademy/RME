#ifndef RME_RENDERING_CORE_TEXT_RENDERER_H_
#define RME_RENDERING_CORE_TEXT_RENDERER_H_

#include <string>
#include <glm/glm.hpp>

struct NVGcontext;

class TextRenderer {
public:
	static void LoadFont(NVGcontext* vg);
	static void BeginFrame(NVGcontext* vg, int width, int height, float pixelRatio);
	static void EndFrame(NVGcontext* vg);

	static void DrawText(NVGcontext* vg, int x, int y, const std::string& text, const glm::vec4& color = glm::vec4(1.0f), float fontSize = 16.0f);
	static void DrawTextBox(NVGcontext* vg, int x, int y, int width, const std::string& text, const glm::vec4& color = glm::vec4(1.0f), float fontSize = 16.0f);

	static void DrawRect(NVGcontext* vg, int x, int y, int w, int h, const glm::vec4& color);
};

#endif // RME_RENDERING_CORE_TEXT_RENDERER_H_
