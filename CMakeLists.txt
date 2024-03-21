cmake_minimum_required(VERSION 3.24)
project(vulkan)
project(vkMMD)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(SPDLOG_USE_STD_FORMAT ON)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    if (WIN32)
        set(CMAKE_WIN32_EXECUTABLE ON)
    endif()
endif()

set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(taskflow_src ${FETCHCONTENT_BASE_DIR}/cpp_taskflow-src)
include(FetchContent)

include_directories(
        external
        imgui
        pch
        saba/src
        saba/example
        external/cpp_taskflow-src

        src/vkH
        src/vkM
)

set(texPath "${CMAKE_SOURCE_DIR}/src/pictures/")
set(shaderPath "${CMAKE_SOURCE_DIR}/src/shaders/")
set(imguiPath "${CMAKE_SOURCE_DIR}/src/vkH/editor/")
configure_file("${CMAKE_SOURCE_DIR}/pch/config.h.in" "${CMAKE_SOURCE_DIR}/pch/config.h")

find_package(Vulkan REQUIRED)
find_library(glfw_msvc glfw3 PATHS ${CMAKE_SOURCE_DIR}/lib/msvc)
find_library(glfw_mingw glfw3 PATHS ${CMAKE_SOURCE_DIR}/lib/mingw)
find_program(glsl glslc REQUIRED)

if (NOT EXISTS "${taskflow_src}")
    FetchContent_Declare(
            cpp_taskflow
            GIT_REPOSITORY      https://github.com/taskflow/taskflow.git
            GIT_TAG             v3.6.0
            #        SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/external/cpp_taskflow"
            #        BINARY_DIR     "${CMAKE_CURRENT_BINARY_DIR}/external_build/cpp_taskflow"
    )
    FetchContent_MakeAvailable(cpp_taskflow)
endif ()

file(GLOB_RECURSE yic_srcs "src/*.h" "src/*.cpp")
file(GLOB_RECURSE vertex ${CMAKE_SOURCE_DIR}/src/shaders/*.vert)
file(GLOB_RECURSE fragment ${CMAKE_SOURCE_DIR}/src/shaders/*.frag)
file(GLOB_RECURSE pch_files "pch/*.h" "pch/*.cpp")

foreach (vert_shader ${vertex})
    get_filename_component(shader_name ${vert_shader} NAME_WE)
    execute_process(COMMAND ${glsl} ${vert_shader} -o ${CMAKE_CURRENT_BINARY_DIR}/${shader_name}.spv)
endforeach ()

foreach (frag_shader ${fragment})
    get_filename_component(shader_name ${frag_shader} NAME_WE)
    execute_process(COMMAND ${glsl} ${frag_shader} -o ${CMAKE_CURRENT_BINARY_DIR}/${shader_name}.spv)
endforeach ()

add_executable(vulkan main.cpp ${yic_srcs})
add_executable(vkMMD vkMMD.cpp ${yic_srcs})

target_precompile_headers(vulkan PRIVATE ${pch_files})
target_precompile_headers(vkMMD PRIVATE ${pch_files})

if(MSVC)
    target_link_libraries(vulkan ${glfw_msvc} "Vulkan::Vulkan" "imgui" "Saba" )
    target_include_directories(vulkan PRIVATE ${CMAKE_BINARY_DIR})
    target_include_directories(vulkan PRIVATE external/spdlog)
    target_include_directories(vulkan PRIVATE taskflow)

    target_link_libraries(vkMMD ${glfw_msvc} "Vulkan::Vulkan" "Saba" "imgui")
elseif (MINGW)
    target_link_libraries(vulkan ${glfw_mingw} "Vulkan::Vulkan" "imgui" "Saba" )
    target_include_directories(vulkan PRIVATE ${CMAKE_BINARY_DIR})
    target_include_directories(vulkan PRIVATE external/spdlog)
    target_include_directories(vulkan PRIVATE taskflow)

    target_link_libraries(vkMMD ${glfw_mingw} "Vulkan::Vulkan" "Saba" "imgui")
    target_include_directories(vkMMD PRIVATE external/spdlog)
else ()
    message(FATAL_ERROR "unsupported compiler")
endif ()

file(INSTALL saba/viewer/Saba/Viewer/resource DESTINATION ${CMAKE_BINARY_DIR})

add_subdirectory(imgui)
add_subdirectory(saba)









