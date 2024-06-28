#version 460

layout(location = 0)out vec2 texCoords;

const vec2 vertices[6] = vec2[](
    vec2(-1.0, -1.0),  // 第一个三角形
    vec2(1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, 1.0),   // 第二个三角形
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

const vec2 triangles[3] = vec2[](
    vec2(0.0f, -0.5f),
    vec2(0.5f, 0.5f),
    vec2(-0.5f, 0.5f)
);

const vec2 texCoordsData[6] = vec2[](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0)
);

void main() {
    gl_Position = vec4(vertices[gl_VertexIndex], 0.0, 1.0);
    texCoords = texCoordsData[gl_VertexIndex];
}