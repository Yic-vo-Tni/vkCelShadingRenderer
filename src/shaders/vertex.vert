#version 450

layout(location = 0) in vec2 vertexPositions;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform Camera{
    mat4 viewProj;
} Camera_data;

layout(binding = 1) readonly buffer storageBuffer {
    mat4 model[];
} m_matrix;

void main() {
    gl_Position = Camera_data.viewProj * m_matrix.model[gl_InstanceIndex] * vec4(vertexPositions, 0.0, 1.0);
    fragColor = vertexColor;
    fragTexCoord = vertexTexCoord;
}