//
// Created by lenovo on 11/16/2023.
//

#ifndef VULKAN_NONCOPYABLE_H
#define VULKAN_NONCOPYABLE_H

#include "yic_pch.h"

namespace yic{

    class nonCopyable{
    public:
        nonCopyable() = default;
        ~nonCopyable() = default;
        nonCopyable(const nonCopyable&) = delete;
        nonCopyable& operator=(const nonCopyable&) = delete;
    };

    template <typename T>
    class Singleton{
    public:
        Singleton() = default;
        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        template<typename...Args>
        static T* get(Args&&... args){
            static T singleton{std::forward<Args>(args)...};
            return &singleton;
        }
    };

#define vkGet inline static constexpr
}

#endif //VULKAN_NONCOPYABLE_H






























//        template<template<typename> class PtrType, class... Args>
//        static PtrType<T> createInstance(Args&&... args) {
//            auto& instance = getInstance<PtrType>();
//            if (!instance) {
//                if constexpr (std::is_same<PtrType<T>, std::unique_ptr<T>>::value) {
//                    instance = std::make_unique<T>(std::forward<Args>(args)...);
//                } else if constexpr (std::is_same<PtrType<T>, std::shared_ptr<T>>::value) {
//                    instance = std::make_shared<T>(std::forward<Args>(args)...);
//                } else {
//                    static_assert(false, "PtrType must be either std::unique_ptr or std::shared_ptr");
//                }
//            }
//            return instance; // 返回一个新的智能指针实例
//        }
//
//        template<template<typename> class PtrType>
//        static PtrType<T>& getInstance() {
//            static PtrType<T> instance_;
//            return instance_;
//        }

