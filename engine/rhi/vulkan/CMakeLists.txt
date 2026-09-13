find_package(Vulkan REQUIRED)

add_library(engine_rhi_vulkan
    src/vulkan_device.cpp

    src/instance.cpp
    src/enumerate.cpp
    src/filter.cpp
)

target_include_directories(engine_rhi_vulkan
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_rhi_vulkan
    PUBLIC  engine::rhi::interface
    PUBLIC  Vulkan::Vulkan
)

target_compile_definitions(engine_rhi_vulkan PRIVATE LUMEN_BUILD_DLL)

add_library(engine::rhi::vulkan ALIAS engine_rhi_vulkan)
