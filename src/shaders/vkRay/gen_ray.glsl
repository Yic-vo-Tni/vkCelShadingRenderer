#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "common_ray.glsl"
#include "host_device.h"

layout(location = 0) rayPayloadEXT hitPayload prd;

layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
layout(set = 0, binding = 1, rgba32f) uniform image2D image;
layout(set = 1, binding = 0) uniform _GlobalUniforms { GlobalUniforms uni; };
layout(push_constant) uniform _PushConstantRay { PushConstantRay pcRay; };

void main(){
    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV = pixelCenter / vec2(gl_LaunchSizeEXT.xy);
    vec2  d = inUV * 2.0 - 1.0;

    vec4 origin = uni.viewInverse * vec4(0, 0, 0, 1);
    vec4 target = uni.projInverse * vec4(d.x, d.y, 1, 1);
    vec4 direction = uni.viewInverse * vec4(normalize(target.xyz), 0);

    uint rayFlags = gl_RayFlagsOpaqueEXT;
    float tMin = 0.001;
    float tMax = 10000.0;

    traceRayExt(tlas, rayFlags, 0xFF, 0, 0, 0, origin.xyz, tMax, 0);

    imageStore(image, ivec2(gl_LaunchIDEXT.xy), vec4(prd.hitValue, 1.f));

}