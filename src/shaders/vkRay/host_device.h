//
// Created by lenovo on 3/28/2024.
//

#ifndef VKMMD_HOST_DEVICE_H
#define VKMMD_HOST_DEVICE_H

#ifdef __cplusplus
#include <glm/glm.hpp>
// GLSL Type
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

struct ObjDesc
{
    int      txtOffset;             // Texture index offset in the array of textures
    uint64_t vertexAddress;         // Address of the Vertex buffer
    uint64_t indexAddress;          // Address of the index buffer
    uint64_t materialAddress;       // Address of the material buffer
    uint64_t materialIndexAddress;  // Address of the triangle material index buffer
};

// Uniform buffer set at each frame
struct GlobalUniforms
{
    mat4 viewProj;     // Camera view * projection
    mat4 viewInverse;  // Camera inverse view matrix
    mat4 projInverse;  // Camera inverse projection matrix
};

// Push constant structure for the raster
struct PushConstantRaster
{
    mat4  modelMatrix;  // matrix of the instance
    vec3  lightPosition;
    uint  objIndex;
    float lightIntensity;
    int   lightType;
};


// Push constant structure for the ray tracer
struct PushConstantRay
{
    vec4  clearColor;
    vec3  lightPosition;
    float lightIntensity;
    int   lightType;
};

struct Vertex  // See ObjLoader, copy of VertexObj, could be compressed for device
{
    vec3 pos;
    vec3 nrm;
    vec3 color;
    vec2 texCoord;
};

struct WaveFrontMaterial  // See ObjLoader, copy of MaterialObj, could be compressed for device
{
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    vec3  transmittance;
    vec3  emission;
    float shininess;
    float ior;       // index of refraction
    float dissolve;  // 1 == opaque; 0 == fully transparent
    int   illum;     // illumination model (see http://www.fileformat.info/format/material/)
    int   textureId;
};



#endif //VKMMD_HOST_DEVICE_H
