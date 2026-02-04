#include "rendering/postprocess/post_process_manager.h"

namespace {

	const char* screen_frag = R"(
#version 450 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    FragColor = texture(u_Texture, vTexCoord);
}
)";

	// Auto-register (Bilinear/Screen)
	struct ScreenRegister {
		ScreenRegister() {
			PostProcessManager::Instance().Register(ShaderNames::NONE, screen_frag);
		}
	} screen_register;

} // namespace
