cmake_minimum_required(VERSION 3.23)
project(engine)

set(LUMEN_ENGINE_NAME "lumen_engine")
set(LUMEN_ENGINE_BINARY_NAME "lumen_engine")

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


add_library(${LUMEN_ENGINE_BINARY_NAME}
    src/core/application.cpp
)

target_include_directories(${LUMEN_ENGINE_BINARY_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/${LUMEN_ENGINE_NAME}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(${LUMEN_ENGINE_BINARY_NAME}
    PUBLIC
        ${LUMEN_PLATFORM_DEF}
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(${LUMEN_ENGINE_BINARY_NAME}
        PUBLIC
            LUMEN_DYNAMIC_LINK
        PRIVATE
            LUMEN_BUILD_DLL
    )
else()
    target_compile_definitions(${LUMEN_ENGINE_BINARY_NAME}
        PUBLIC
            LUMEN_STATIC_LINK
    )
endif()

target_compile_features(${LUMEN_ENGINE_BINARY_NAME} PUBLIC cxx_std_23)
