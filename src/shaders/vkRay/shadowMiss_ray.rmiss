#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 1) rayPayloadEXT bool isShadowed;

void main(){
    isShadowed = false;
}