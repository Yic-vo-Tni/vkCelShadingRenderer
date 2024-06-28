#version 460

layout(location = 0) in vec3 Pos;

layout(set = 0, binding = 0) uniform MVP{
    mat4 mvp;
} Mvp;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = Mvp.mvp * vec4(Pos, 1.f);

    fragColor = vec3(1.f, 0.1f, 0.1f);
}