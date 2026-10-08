find_package(volk CONFIG REQUIRED)
find_package(VulkanUtilityLibraries CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_library(engine_vulkan_core STATIC
    src/instance.cpp
    src/physical_device.cpp
    src/device.cpp
    src/vma_allocator.cpp
)

target_include_directories(engine_vulkan_core
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_vulkan_core PUBLIC
    engine::core
    
    volk::volk
    Vulkan::UtilityHeaders
    GPUOpen::VulkanMemoryAllocator
)

target_compile_definitions(engine_vulkan_core
    PUBLIC
        VK_NO_PROTOTYPES
)

add_library(engine::vulkan_core ALIAS engine_vulkan_core)
