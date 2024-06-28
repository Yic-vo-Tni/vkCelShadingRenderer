#version 460

layout(set = 0, binding = 0) uniform sampler3D Tex;

layout(location = 0) in vec2 texCoords;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 pos = vec3(texCoords, 0.f);
    vec3 dir = normalize(vec3(0.f, 0.f, -1.f));
    float totalDensity = 0.f;

    for(int i = 0; i < 50; i++){
        float density = texture(Tex, pos).r;
//        totalDensity += density * 0.01f;
        totalDensity += density * 0.008f;

        pos += dir * 0.02f;
    }

    float adjustedDensity = 1.f - exp(-totalDensity * 3.f);

    vec3 color = vec3(1.f);
    outColor = vec4(color, adjustedDensity);
}
