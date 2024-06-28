#version 460

layout(location = 0) in vec2 texCoords;
layout(location = 0) out vec4 fragColor;

layout(set = 0,binding = 0) uniform sampler2D depthTexture;

float unpackDepth24(uint packedDepth) {
    return float(packedDepth & 0xFFFFFF) / float(0xFFFFFF);
}

float linearizedDepth(float depth, float near, float far)
{
    return near * far / (far + depth * (near - far));
}

void main() {
    float depth = texture(depthTexture, texCoords).r;
    if (depth >= 0.999) {
        fragColor = vec4(0.9, 0.9, 0.9, 0.9);
    } else {
        depth = depth * 1000 - 990;
        depth = depth / 10;

        fragColor = vec4(vec3(depth), 1.0);
    }

    //fragColor = vec4(vec3(linearizedDepth(depth, 0.1f, 500.f)), 1.f);
}