cmake_minimum_required(VERSION 3.23)
project(sandbox)

find_package(volk CONFIG REQUIRED)
find_package(VulkanUtilityLibraries CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    src/sandbox_app.cpp
    src/vulkan_render_layer.cpp
    src/platform.cpp
    src/check_vk.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        lumen_engine

        volk::volk
        Vulkan::UtilityHeaders
        GPUOpen::VulkanMemoryAllocator
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
endif()
