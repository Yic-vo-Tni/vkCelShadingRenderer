#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform MVP{
    mat4 mvp;
}Mvp;

void main(){
    gl_Position = Mvp.mvp * vec4(pos, 1.f);
    fragColor = vec4(color, 1.f);
}