//
// Created by lenovo on 11/25/2023.
//

#ifndef VULKAN_YIC_TEMPLATE_H
#define VULKAN_YIC_TEMPLATE_H

#include "yic_pch.h"
#include <type_traits>

template<typename T>
struct is_vector : std::false_type {};

template<typename T>
struct is_vector<std::vector<T>> : std::true_type {};

template<typename T>
struct is_DeviceMemory : std::false_type {};
template<>
struct is_DeviceMemory<vk::DeviceMemory> : std::true_type {};

template<typename T, typename... Types>
struct is_any_of;


template<typename T>
struct is_any_of<T> : std::false_type {};


template<typename T, typename First, typename... Rest>
struct is_any_of<T, First, Rest...>
        : std::conditional<std::is_same<T, First>::value, std::true_type, is_any_of<T, Rest...>>::type {};


template<typename T, typename Resource>
auto destroyResource(T t, Resource& resource)
-> typename std::enable_if<is_vector<Resource>::value>::type {
    for (auto& item : resource) {
        t.destroy(item);
    }
}

template<typename T, typename Resource>
auto destroyResource(T t, Resource& resource)
-> typename std::enable_if<!is_vector<Resource>::value && !is_DeviceMemory<Resource>::value>::type {
    t.destroy(resource);
}

template<typename T, typename Resource>
auto destroyResource(T t, Resource& resource)
-> typename std::enable_if<!is_vector<Resource>::value && is_any_of<Resource, vk::DeviceMemory>::value>::type {
    t.free(resource);
}

template<typename T, typename... Resources>
void vkDestroyAll(T t, Resources&... resources) {
    (destroyResource(t, resources), ...);
}

template<typename T>
void vkRest(T& resource) {
    resource.reset();
}


template<typename First, typename... Rest>
void vkRest(First& first, Rest&... rest) {
    first.reset();
    vkRest(rest...);
}


#endif //VULKAN_YIC_TEMPLATE_H




















