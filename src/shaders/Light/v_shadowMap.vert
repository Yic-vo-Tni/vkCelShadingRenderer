#version 450

layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inNormal;
//layout(location = 2) in vec2 inUV;

//layout(set = 0, binding = 0) uniform UBO {
//    mat4 lightViewMatrix;
//    mat4 lightProjMatrix;
//} ubo;

layout(set = 0, binding = 0) uniform UBO {
    mat4 lightMVPMatrix;
} ubo;

//layout(location = 0) out float lightSpaceViewDistance;

void main() {
//    vec4 lightSpacePos = ubo.lightMVPMatrix * vec4(inPosition, 1.f);
//    lightSpaceViewDistance = length(lightSpacePos.xyz);
    vec4 worldPosition = vec4(inPosition, 1.0);
    gl_Position = ubo.lightMVPMatrix * worldPosition;
}