cmake_minimum_required(VERSION 3.23)
project(engine)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)

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

add_library(engine_common INTERFACE)
target_compile_definitions(engine_common INTERFACE
    ${LUMEN_PLATFORM_DEF}
)
target_compile_features(engine_common INTERFACE cxx_std_23)
add_library(engine::common ALIAS engine_common)

add_subdirectory(core)
add_subdirectory(runtime)
add_subdirectory(vulkan_core)
add_subdirectory(gfx)

add_library(lumen_engine INTERFACE)
target_link_libraries(lumen_engine INTERFACE
    engine::core
    engine::runtime
    engine::vulkan_core
    engine::gfx
)
target_include_directories(lumen_engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(lumen::engine ALIAS lumen_engine)
