#ifndef RME_RENDERING_CORE_TEXT_RENDERER_H_
#define RME_RENDERING_CORE_TEXT_RENDERER_H_

#include <string>
#include <glm/glm.hpp>

struct NVGcontext;

class TextRenderer {
public:
	static void Init();
	static void Shutdown();
	static void BeginFrame(int width, int height);
	static void EndFrame();

	static void DrawText(int x, int y, const std::string& text, const glm::vec4& color = glm::vec4(1.0f), float fontSize = 16.0f);
	static void DrawTextBox(int x, int y, int width, const std::string& text, const glm::vec4& color = glm::vec4(1.0f), float fontSize = 16.0f);

	static void DrawRect(int x, int y, int w, int h, const glm::vec4& color);

	static NVGcontext* GetContext() {
		return nvgContext;
	}

private:
	static NVGcontext* nvgContext;
	static int fontNormal;
};

#endif // RME_RENDERING_CORE_TEXT_RENDERER_H_
