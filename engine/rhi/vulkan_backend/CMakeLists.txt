find_package(Vulkan REQUIRED)

add_library(engine_rhi_vulkan_backend
    src/vulkan_device.cpp

    src/instance.cpp
    src/enumerate.cpp
    src/filter.cpp
    src/logical_device.cpp
    src/debug_utils_messenger.cpp
    src/buffer.cpp
    src/surface.cpp
    src/physical_device.cpp
    src/vulkan_translate.cpp
    src/texture.cpp
)

target_include_directories(engine_rhi_vulkan_backend
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_rhi_vulkan_backend
    PUBLIC  engine::rhi::interface
    PUBLIC  Vulkan::Vulkan
)

target_compile_definitions(engine_rhi_vulkan_backend PRIVATE LUMEN_BUILD_DLL)

add_library(engine::rhi::vulkan_backend ALIAS engine_rhi_vulkan_backend)
