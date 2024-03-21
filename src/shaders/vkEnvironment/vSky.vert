#version 450

layout (set = 0, binding = 0) uniform cameraVectors {
    //    vec4 forwards;
    //    vec4 right;
    //    vec4 up;

    mat4 vpMatrix;
} camera;

layout (location = 0) out vec3 forwards;

//const vec2 screenCorners[6] = {
//    vec2(-1.f, -1.f),
//    vec2(-1.f,  1.f),
//    vec2( 1.f,  1.f),
//    vec2( 1.f,  1.f),
//    vec2( 1.f, -1.f),
//    vec2(-1.f, -1.f)
//};

const vec3 cubeVertices[36] = {
    // Front Face
    vec3(-1.f, -1.f, 1.f),
    vec3(-1.f, 1.f, 1.f),
    vec3(1.f, 1.f, 1.f),
    vec3(1.f, 1.f, 1.f),
    vec3(1.f, -1.f, 1.f),
    vec3(-1.f, -1.f, 1.f),

    // Back Face
    vec3(-1.f, -1.f, -1.f),
    vec3(-1.f, 1.f, -1.f),
    vec3(1.f, 1.f, -1.f),
    vec3(1.f, 1.f, -1.f),
    vec3(1.f, -1.f, -1.f),
    vec3(-1.f, -1.f, -1.f),

    // Left Face
    vec3(-1.f, -1.f, -1.f),
    vec3(-1.f, 1.f, -1.f),
    vec3(-1.f, 1.f, 1.f),
    vec3(-1.f, 1.f, 1.f),
    vec3(-1.f, -1.f, 1.f),
    vec3(-1.f, -1.f, -1.f),

    // Right Face
    vec3(1.f, -1.f, -1.f),
    vec3(1.f, 1.f, -1.f),
    vec3(1.f, 1.f, 1.f),
    vec3(1.f, 1.f, 1.f),
    vec3(1.f, -1.f, 1.f),
    vec3(1.f, -1.f, -1.f),

    // Top Face
    vec3(-1.f, 1.f, -1.f),
    vec3(-1.f, 1.f, 1.f),
    vec3(1.f, 1.f, 1.f),
    vec3(1.f, 1.f, 1.f),
    vec3(1.f, 1.f, -1.f),
    vec3(-1.f, 1.f, -1.f),

    // Bottom Face
    vec3(-1.f, -1.f, -1.f),
    vec3(-1.f, -1.f, 1.f),
    vec3(1.f, -1.f, 1.f),
    vec3(1.f, -1.f, 1.f),
    vec3(1.f, -1.f, -1.f),
    vec3(-1.f, -1.f, -1.f)
};

void main() {
    //    vec2 pos = screenCorners[gl_VertexIndex];
    //    gl_Position = camera.vpMatrix * vec4(pos, 0.f, 1.f);
    //    forwards = normalize(camera.forwards + pos.x * camera.right - pos.y * camera.up).xyz;

    vec3 pos = cubeVertices[gl_VertexIndex];
    gl_Position = camera.vpMatrix * vec4(pos, 1.f);
    forwards = pos;
}

