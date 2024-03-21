//
// Created by lenovo on 11/11/2023.
//

#ifndef YIC_VULKAN_YIC_PCH_H
#define YIC_VULKAN_YIC_PCH_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "vulkan/vulkan.hpp"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define	STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include <stb/stb_image.h>
/////////////////////////////

#include "iostream"
#include "stdexcept"
#include "exception"
#include "vector"
#include "set"
#include "optional"
#include "limits"
#include "chrono"
#include "memory"
#include "fstream"
#include "bitset"
#include "unordered_map"
#include "variant"
#include "tuple"
#include "mutex"
#include "functional"
#include "map"
#include "queue"
#include "random"
#include "sstream"
#include "future"

/////////////////////////////

#ifdef NDEBUG
    const bool enableDebug = false;
#else
    const bool enableDebug = true;
#endif

#define vkDebug  if(enableDebug)

extern void vkCreate(const std::function<void()>& fun, const std::string& description, int n = -1);

#define vkTest(x) std::cout << "test" << x << std::endl;


#endif //YIC_VULKAN_YIC_PCH_H
