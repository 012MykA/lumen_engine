find_package(Vulkan REQUIRED)

add_library(engine_rhi_vulkan_backend
    src/vulkan_rendering_device.cpp

    src/vulkan_instance.cpp
    src/vulkan_enumerate.cpp
    src/vulkan_filter.cpp
    src/vulkan_logical_device.cpp
    src/vulkan_debug_utils_messenger.cpp
    src/vulkan_buffer.cpp
    src/vulkan_surface.cpp
    src/vulkan_physical_device.cpp
    src/vulkan_translate.cpp
    src/vulkan_texture.cpp
    src/vulkan_sampler.cpp
    src/vulkan_shader_module.cpp
    src/vulkan_pipeline.cpp
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
