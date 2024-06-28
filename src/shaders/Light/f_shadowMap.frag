#version 450

layout(location = 0) out vec4 outColor;

//layout(location = 0) in float lightSpaceViewDistance;

void main(){
//    float normalizedDis = lightSpaceViewDistance / 1000.f;
    outColor = vec4(0.f, 0.f, 0.f, 1.f);
}