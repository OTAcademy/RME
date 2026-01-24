#version 450 core

in vec3 TexCoord;
in vec4 Tint;

out vec4 FragColor;

uniform sampler2DArray uAtlas;
uniform vec4 uGlobalTint;

void main() {
    vec4 texel = texture(uAtlas, TexCoord);
    FragColor = texel * Tint * uGlobalTint;
}
