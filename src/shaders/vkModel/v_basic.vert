#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragPosLightSpace;

layout(set = 0, binding = 0) uniform MVP {
    mat4 mvp;
} Mvp;

layout(set = 0, binding = 3) uniform LightVp{
    mat4 vp;
} lightMatrix;

void main() {
    fragPosLightSpace = lightMatrix.vp * vec4(inPos, 1.f);
    gl_Position = Mvp.mvp * vec4(inPos, 1.0);
    fragUV = inUV;
}