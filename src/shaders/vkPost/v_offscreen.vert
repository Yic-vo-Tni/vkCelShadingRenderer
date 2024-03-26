#version 450

layout(location = 0) out vec2 texCoords;

void main() {
    vec2 posI = vec2((gl_VertexIndex & 1) * 2.0 - 1.0, (gl_VertexIndex & 2) - 1.0);
    vec2 pos = vec2(posI);
    texCoords = pos * 0.5 + 0.5;
    gl_Position = vec4(pos, 0.0, 1.0);
}