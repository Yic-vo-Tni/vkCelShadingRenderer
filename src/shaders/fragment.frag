#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D material;

void main() {
    outColor = mix(vec4(fragColor.rgb, 1.0), texture(material, fragTexCoord), 0.8f);
}