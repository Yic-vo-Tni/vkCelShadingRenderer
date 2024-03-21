#version 450

layout(location = 0) in vec2 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform camera{
    mat4 view;
    mat4 projection;
    mat4 viewPorjection;
} cameraData;

layout(std430, set = 0, binding = 1) readonly buffer storageBuffer{
    mat4 model[];
} objectData;

void main() { 
    gl_Position = cameraData.viewPorjection * objectData.model[gl_InstanceIndex] * vec4(vertexPosition, 0.0, 1.0);
    fragColor = vertexColor;
    fragTexCoord = vertexTexCoord;
}

//在 Vulkan（以及OpenGL）中，针对着色器的 Uniform Buffer Objects (UBO) 和 Storage Buffer Objects (SBO) 的内存布局，有几种标准布局规范。其中最常用的是 std140 和 std430。这些布局规范定义了如何在内存中排列着色器的 uniform 或 storage 变量，以确保着色器能正确理解 CPU 端提供的数据结构。
//
//std140
//用途：std140 主要用于 uniform buffer 的内存布局。
//特点：它有着较严格的对齐规则，旨在确保在不同的硬件和驱动上有一致的行为。
//对齐规则：
//标量类型（如 float、int）对齐到 4 字节。
//向量类型对齐到它们的组件大小的 4 倍（例如，vec3 对齐到 16 字节）。
//矩阵按照其列（或行，如果使用行主序）对齐，每列（或行）视为一个向量。
//数组元素对齐到数组元素类型的对齐长度的倍数，且通常比单个元素的对齐长度大。
//结构体的对齐是其最大成员的对齐的倍数，并且结构体的每个成员也按照这个规则对齐。
//std430
//用途：std430 通常用于 storage buffer 的内存布局。
//特点：它相对于 std140 有更紧凑的内存布局，允许更高效的内存使用，但在跨平台兼容性方面可能略逊于 std140。
//对齐规则：
//标量和向量类型的对齐与其自身大小相同（即，vec3 对齐到 12 字节）。
//矩阵和数组的对齐规则与 std140 相同，但由于向量对齐的不同，整体上更为紧凑。
//内存对齐的重要性
//保证一致性：确保无论在哪种硬件或驱动上，着色器都能正确地解释传递给它的数据。
//提高性能：正确的对齐可以提高内存访问的效率，尤其是在 GPU 上。
//实际应用
//在实际编程时，需要根据具体的使用场景和性能要求选择合适的内存布局规范。std140 由于其简单和一致性，适合大多数常规用途，尤其是当需要在不同的系统和硬件上保持一致行为时。而在对内存布局的紧凑性有更高要求时，可以考虑使用 std430。在实现时，您需要在 GLSL 着色器代码中显式指定所使用的布局规范