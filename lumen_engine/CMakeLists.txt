cmake_minimum_required(VERSION 3.23)
project(lumen_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE ENGINE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lumen_engine/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lumen_engine/*.hpp
)
add_library(lumen_engine STATIC ${ENGINE_SOURCES})

# Platform
if(WIN32)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Windows x86 is not supported!")
    endif()
    set(LUMEN_PLATFORM_DEF LUMEN_PLATFORM_WINDOWS)
elseif(APPLE)
    message(FATAL_ERROR "Apple platform is not supported!")
elseif(ANDROID)
    message(FATAL_ERROR "Android platform is not supported!")
elseif(UNIX)
    set(LUMEN_PLATFORM_DEF LUMEN_PLATFORM_LINUX)
else()
    message(FATAL_ERROR "Unknown platform!")
endif()

target_compile_definitions(lumen_engine
    PUBLIC
        ${LUMEN_PLATFORM_DEF}
        VK_NO_PROTOTYPES

        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_RADIANS
)

find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(VulkanUtilityLibraries CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(Stb REQUIRED)

target_link_libraries(lumen_engine
    PUBLIC
        spdlog::spdlog
        glfw
        glm::glm
        volk::volk
        Vulkan::UtilityHeaders
        GPUOpen::VulkanMemoryAllocator
)

target_include_directories(lumen_engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src

    ${Stb_INCLUDE_DIR}
)
