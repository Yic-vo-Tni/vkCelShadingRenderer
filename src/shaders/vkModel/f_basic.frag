#version 460

layout(set = 0, binding = 1) uniform sampler2D texSampler;
layout(set = 0, binding = 2) uniform sampler2D shadowMap;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragPosLightSpace;

layout(std140, set = 0, binding = 4) uniform shadowMapConfig{
    float texelSize;
    float bias;
    float shadow;

    bool enable_pcf;
    int pcf_para;
    int pcf;

    float posCameraX;
    float posCameraY;
    float posCameraZ;
    float colorLightX;
    float colorLightY;
    float colorLightZ;

} shadowConfig;

float shadowCalculation(vec4 fragPosLightSpace)
{
    float shadow = 0.f;

    if(!shadowConfig.enable_pcf){
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;


        float closestDepth = texture(shadowMap, projCoords.xy).r;
        float currentDepth = fragPosLightSpace.z;

        shadow = currentDepth > closestDepth + shadowConfig.bias ? shadowConfig.shadow : 0.0;
    } else{
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        projCoords = projCoords * 0.5 + 0.5;

        for (int x = -shadowConfig.pcf_para; x <= shadowConfig.pcf_para; x++) {
            for (int y = -shadowConfig.pcf_para; y <= shadowConfig.pcf_para; y++) {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * shadowConfig.texelSize).r;
                shadow += fragPosLightSpace.z > pcfDepth + shadowConfig.bias ? shadowConfig.shadow : 0.0;
            }
        }

        shadow /= shadowConfig.pcf;
    }

    return shadow;
}

void main()
{
    vec2 offset = 1.f / textureSize(texSampler, 0);
    vec3 center = texture(texSampler, fragUV).rgb;
    vec3 right = texture(texSampler, fragUV + vec2(offset.x, 0.f)).rgb;
    vec3 left = texture(texSampler, fragUV - vec2(offset.x, 0.f)).rgb;
    vec3 top = texture(texSampler, fragUV + vec2(0.f, offset.y)).rgb;
    vec3 bottom = texture(texSampler, fragUV - vec2(0.f, offset.y)).rgb;

    // Calculate average for edge detection
    vec3 fartherRight = texture(texSampler, fragUV + vec2(offset.x * 2.0, 0.0)).rgb;
    vec3 fartherLeft = texture(texSampler, fragUV - vec2(offset.x * 2.0, 0.0)).rgb;
    vec3 fartherTop = texture(texSampler, fragUV + vec2(0.0, offset.y * 2.0)).rgb;
    vec3 fartherBottom = texture(texSampler, fragUV - vec2(0.0, offset.y * 2.0)).rgb;

    vec3 average = (right + left + top + bottom + fartherRight + fartherLeft + fartherTop + fartherBottom) / 8.0;
    vec3 edge = center - average;
    float edgeStrength = length(edge) * 4.0;  // Enhance edge strength

    // Calculate darker edge color
    vec3 darkerEdgeColor = center * 0.3;  // Make the edge color darker
    //vec3 darkerEdgeColor = vec3(1.f, 0.f, 0.f);
    darkerEdgeColor = clamp(darkerEdgeColor, 0.0, 1.0);

    // Mix edge color and original color based on edge strength
    vec3 resultColor = mix(center, darkerEdgeColor, min(edgeStrength, 1.0));

    // Apply a slight overall brightness reduction to original color
    float brightness = 1.1;  // Reduce overall brightness slightly
    resultColor *= brightness;
    resultColor = clamp(resultColor, 0.0, 1.f);

    vec3 color = vec3(resultColor);


    // ?????????
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));

    // ??????????
    float saturation = 2.f;  // ???????????????????

    // ????????
    vec3 greyScale = vec3(luminance);  // ?????????
    vec3 saturatedColor = mix(greyScale, color, saturation); // ??mix????saturation??????????

    color = vec3(saturatedColor);

    float shadowStrength = clamp(edgeStrength, 0.0, 1.0); // ???????0?1??
    float shadowFactor = 1.0 - shadowStrength * 0.2; // ??????????????????

    // ???????????
    vec3 shadowedColor = color * shadowFactor;

    // ??????????
    //float lightDirection = -0.5 * (fragUV.x + fragUV.y);  // ???????????

    // ???????????????
    //vec3 shadowColor = color * (0.5 + 0.5 * lightDirection);

    float shadow = shadowCalculation(fragPosLightSpace);
    shadowedColor = vec3(shadowedColor.rgb * (1.f - shadow));

    outColor = vec4(shadowedColor, 1.0);
}
