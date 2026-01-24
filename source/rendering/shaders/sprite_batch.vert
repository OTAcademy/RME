#version 450 core

// Per-vertex attributes (unit quad)
layout (location = 0) in vec2 aPos;       // 0,0 to 1,1
layout (location = 1) in vec2 aTexCoord;  // 0,0 to 1,1

// Per-instance attributes
layout (location = 2) in vec4 aRect;      // x, y, w, h
layout (location = 3) in vec4 aUV;        // u_min, v_min, u_max, v_max
layout (location = 4) in vec4 aTint;      // r, g, b, a
layout (location = 5) in float aLayer;    // texture array layer

out vec3 TexCoord;
out vec4 Tint;

uniform mat4 uMVP;

void main() {
    // Transform unit quad to screen position
    vec2 pos = aRect.xy + aPos * aRect.zw;
    gl_Position = uMVP * vec4(pos, 0.0, 1.0);
    
    // Interpolate UVs and pass layer
    TexCoord = vec3(mix(aUV.xy, aUV.zw, aTexCoord), aLayer);
    Tint = aTint;
}
