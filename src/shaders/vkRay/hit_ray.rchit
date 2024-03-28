#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "common_ray.glsl"
#include "wavefront.glsl"
//#include "host_device.h"

hitAttributeEXT vec2 attribs;
//作用：hitAttributeEXT是一个特殊的类型，用于存储光线击中几何体时计算出的属性。在这里，它存储的是一个vec2类型的变量attrib，这通常用来存储重心坐标（Barycentric coordinates），它们表示光线击中三角形面内部的精确位置。
//命名和意义：attrib是“attribute”（属性）的简写，指的是与光线击中的几何体相关的数据。在光线追踪的上下文中，这些属性通常包括但不限于重心坐标，它们对于计算击中点的世界坐标、法线或其他插值属性（如纹理坐标）至关重要。
//用途：通过使用attrib中的重心坐标，着色器可以插值计算出光线击中点的具体属性，如位置、法线、纹理坐标等，这对于进一步的着色和光照计算非常有用。

layout(location = 0) rayPayloadInEXT hitPayload prd;  // prd  pre-ray data,存储每一条光线特定的数据，如颜色、光照强度等。它是光线生成着色器、最近命中着色器、任意命中着色器和未命中着色器之间共享数据的关键结构
//rayPayloadInEXT是一个用于传递光线载荷的类型, prd是光线追踪过程中用于存储和传递结果的数据结构，layout(location = 0)指定了其在着色器间通信时的位置
//在最近命中着色器中，prd可以被更新以反映光线与物体交互后的结果，例如计算得到的光照颜色。然后，这个更新后的载荷可以被光线生成着色器访问以更新最终图像
layout(location = 1) rayPayloadEXT bool isShadowed; //在进行光线追踪以计算阴影时，如果检测到从光线击中点向光源方向的光线被其他物体遮挡，则将isShadowed设置为true，表示该击中点处于阴影中。这个信息随后会被用于调整击中点的光照表现，如降低亮度

//光线载荷（Ray Payload）是在光线追踪管线中传递信息的一种方式。它允许你在光线生成着色器、最近命中着色器、任意命中着色器、未命中着色器之间共享数据。这些数据可以是关于光线的颜色、强度，或者其他任何你想在这些着色器间传递的信息。可以把光线载荷想象成一个光线特有的“背包”，当光线在场景中旅行时，它携带这个“背包”并在途中收集信息

//没有In（rayPayloadEXT）：这是用于初始化和更新光线载荷的。在光线生成着色器中，你会将需要的信息填充到这样的载荷中，比如计算得到的颜色、是否命中物体等信息。这些信息随后可以被其他着色器（如最近命中着色器、任意命中着色器）访问和利用。
//有In（rayPayloadInEXT）：这用于在着色器中接收来自前一个阶段的光线载荷数据。例如，在最近命中着色器中，你可以使用这种类型的变量来接收从光线生成着色器传递过来的光线载荷数据，这样就可以基于之前的计算结果进一步处理。

layout(buffer_reference, scalar) buffer Vertices {Vertex v[];};
layout(buffer_reference, scalar) buffer Indices {ivec3 i[];};
layout(buffer_reference, scalar) buffer Materials {WaveFrontMaterial m[];};
layout(buffer_reference, scalar) buffer MatIndices {int i[];};
layout(set = 0, binding = 0) uniform accelerationStructureEXT tlas;
layout(set = 1, binding = 1) buffer ObjDesc_ {
    ObjDesc i[];
 } objDesc;
layout(set = 1, binding = 2) uniform sampler2D textureSamplers[];

layout(push_constant) uniform _PushConstantRay { PushConstantRay pcRay ;};

//Buffer引用与Scalar布局限定符
//layout(buffer_reference, scalar)：这是一个GLSL扩展，允许着色器直接通过引用访问缓冲区中的数据，而不需要通过标准的着色器输入输出。buffer_reference指的是可以直接引用GPU内存中的缓冲区，这对于光线追踪性能优化非常重要。scalar布局限定符指示数据以标量形式组织，这有助于提高内存访问效率。
//数据结构定义
//Vertices, Indices, Materials, MatIndices：这些声明定义了几种不同类型的缓冲区，分别存储顶点数据、索引数据、材质信息和材质索引。Vertex, WaveFrontMaterial, 和 ObjDesc 应该是在别的地方定义的结构体，表示具体的顶点格式、材质属性和对象描述信息。
//
//Vertex v[]; 表示一个顶点数组，每个顶点包含位置、法线、纹理坐标等信息（具体取决于Vertex结构体的定义）。
//ivec3 i[]; 表示索引数据，每个ivec3包含构成一个三角形的三个顶点的索引。
//WaveFrontMaterial m[]; 表示材质信息数组，每个元素包含了材质的相关属性，如漫反射颜色、镜面反射强度等。
//int i[]; 通常用于存储与三角形相关的材质索引。
//ObjDescs：这是一个特殊的缓冲区，用于存储对象描述信息的数组。ObjDesc i[];定义了一个包含多个对象描述的数组，每个对象描述包括了顶点缓冲区、索引缓冲区和材质数据的GPU内存地址。
//
//Uniforms和Sampler
//tlas：一个加速结构（通常是顶层加速结构，Top Level Acceleration Structure），用于光线追踪查询。
//textureSamplers：一个纹理采样器数组，用于访问对象的纹理。在光线追踪着色器中，可以根据材质信息或顶点纹理坐标来采样纹理。
//Push常量
//_PushConstantRay：这是通过推送常量传递给着色器的一小块数据。在这里，PushConstantRay很可能是一个结构体，包含了光源信息、相机参数等对于光线生成和处理至关重要的信息。
//数组和GPU直接取数据
//[]数组：在GLSL中，[]表示数组，数组中的元素可以是结构体实例，表示一系列同类型的数据集合。通过使用缓冲区引用，着色器可以直接从GPU内存中读取这些数组中的数据，而无需通过传统的顶点输入或统一缓冲区间接访问，这对于光线追踪等性能敏感的计算特别有用。

void main()
{
  // Object data
  ObjDesc    objResource = objDesc.i[gl_InstanceCustomIndexEXT]; //使用从ObjDescs缓冲区中获取的索引（gl_InstanceCustomIndexEXT）来访问当前光线命中的对象描述（ObjDesc）。这个索引基于当前实例的ID，它提供了访问当前实例顶点、索引和材料等资源所需的所有信息
  MatIndices matIndices  = MatIndices(objResource.materialIndexAddress);
  Materials  materials   = Materials(objResource.materialAddress);
  Indices    indices     = Indices(objResource.indexAddress);
  Vertices   vertices    = Vertices(objResource.vertexAddress); //分别初始化材质索引(MatIndices)、材质数据(Materials)、三角形的顶点索引(Indices)和顶点数据(Vertices)。这些结构直接从GPU内存中引用数据，使得着色器可以访问当前被击中物体的详细几何和材质信息

  // Indices of the triangle
  ivec3 ind = indices.i[gl_PrimitiveID];

  // Vertex of the triangle
  Vertex v0 = vertices.v[ind.x];
  Vertex v1 = vertices.v[ind.y];
  Vertex v2 = vertices.v[ind.z];  //使用gl_PrimitiveID来索引当前击中的三角形的顶点索引（ind），然后从顶点缓冲区(Vertices)中获取这些顶点的数据

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y); //计算重心坐标

  // Computing the coordinates of the hit position
  const vec3 pos      = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
  const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space
  //利用重心坐标（barycentrics），通过插值计算出光线击中三角形表面的精确位置（pos），并将该位置转换到世界空间（worldPos）

  // Computing the normal at hit position
  const vec3 nrm      = v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z;
  const vec3 worldNrm = normalize(vec3(nrm * gl_WorldToObjectEXT));  // Transforming the normal to world space
  //同样使用重心坐标对顶点的法线进行插值，得到击中点的法线（nrm），并将其转换到世界空间（worldNrm），用于之后的光照计算

  // Vector toward the light
  vec3  L;
  float lightIntensity = pcRay.lightIntensity;
  float lightDistance  = 100000.0;
  // Point light
  if(pcRay.lightType == 0)
  {
    vec3 lDir      = pcRay.lightPosition - worldPos;
    lightDistance  = length(lDir);
    lightIntensity = pcRay.lightIntensity / (lightDistance * lightDistance);
    L              = normalize(lDir);
  }
  else  // Directional light
  {
    L = normalize(pcRay.lightPosition);
  }
  //根据光源类型（点光源或方向光源），计算从击中点到光源的方向。对于点光源，还需要计算光源距离和光照强度的衰减

  // Material of the object
  int               matIdx = matIndices.i[gl_PrimitiveID];
  WaveFrontMaterial mat    = materials.m[matIdx];


  // Diffuse
  vec3 diffuse = computeDiffuse(mat, L, worldNrm);
  if(mat.textureId >= 0)
  {
    uint txtId    = mat.textureId + objDesc.i[gl_InstanceCustomIndexEXT].txtOffset;
    vec2 texCoord = v0.texCoord * barycentrics.x + v1.texCoord * barycentrics.y + v2.texCoord * barycentrics.z;
    diffuse *= texture(textureSamplers[nonuniformEXT(txtId)], texCoord).xyz;
  }

  vec3  specular    = vec3(0);
  float attenuation = 1;

  // Tracing shadow ray only if the light is visible from the surface
  if(dot(worldNrm, L) > 0)
  {
    float tMin   = 0.001;
    float tMax   = lightDistance;
    vec3  origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    vec3  rayDir = L;
    uint  flags  = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    isShadowed   = true;
    traceRayEXT(tlas,  // acceleration structure
                flags,       // rayFlags
                0xFF,        // cullMask
                0,           // sbtRecordOffset
                0,           // sbtRecordStride
                1,           // missIndex
                origin,      // ray origin
                tMin,        // ray min range
                rayDir,      // ray direction
                tMax,        // ray max range
                1            // payload (location = 1)
    );

    if(isShadowed)
    {
      attenuation = 0.3;
    }
    else
    {
      // Specular
      specular = computeSpecular(mat, gl_WorldRayDirectionEXT, L, worldNrm);
    }
  }

  prd.hitValue = vec3(lightIntensity * attenuation * (diffuse + specular));
}