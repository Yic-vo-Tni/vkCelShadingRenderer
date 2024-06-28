#version 460

layout(location = 0) in vec3 Pos;

layout(set = 0, binding = 0) uniform MVP{
    mat4 mvp;
} Mvp;

layout(location = 0) out vec3 fragColor;

vec2 position[3] = vec2[](
        vec2(0.f, -0.5f),
        vec2(0.5f, 0.5f),
        vec2(-0.5f, 0.5f)
);

void main() {
    gl_Position = Mvp.mvp * vec4(Pos, 1.f);
    //gl_Position = vec4(Pos, 1.f);

    //gl_Position = Mvp.mvp * vec4(position[gl_VertexIndex], 0.f, 1.f);
    //gl_Position = Mvp.mvp * vec4(position[gl_VertexIndex], 0.f, 1.f);
    fragColor = vec3(1.f, 1.f, 1.f);
}