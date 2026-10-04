cmake_minimum_required(VERSION 3.23)
project(engine)

option(BUILD_SHARED_LIBS "Build libraries as shared" ON)

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
    $<$<BOOL:${BUILD_SHARED_LIBS}>:LUMEN_DYNAMIC_LINK>
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:LUMEN_STATIC_LINK>
)
target_compile_features(engine_common INTERFACE cxx_std_23)
add_library(engine::common ALIAS engine_common)

add_subdirectory(core)
add_subdirectory(events)
add_subdirectory(windowing)
add_subdirectory("input")
add_subdirectory(logging)
add_subdirectory(app)
add_subdirectory(vulkan_core)

add_library(lumen_engine INTERFACE)
target_link_libraries(lumen_engine INTERFACE
    engine::core
    engine::events
    engine::windowing
    engine::input
    engine::logging
    engine::app
    engine::vulkan_core
)
target_include_directories(lumen_engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(lumen::engine ALIAS lumen_engine)
