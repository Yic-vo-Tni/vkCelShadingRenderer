#version 450

layout(location = 0) in vec2 texCoords;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D skyTexture;
//layout(set = 0, binding = 1) uniform sampler2D envTexture;

void main(){
    vec4 skyColor = texture(skyTexture, texCoords);
//    vec4 envColor = texture(envTexture, texCoords);

//    outColor = mix(skyColor, envColor, envColor.a);
    outColor = skyColor;
}

