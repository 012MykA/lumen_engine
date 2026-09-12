# Backend platform support
if(WIN32 OR (UNIX AND NOT APPLE))
    set(LUMEN_RHI_VULKAN_SUPPORTED ON)
else()
    set(LUMEN_RHI_VULKAN_SUPPORTED OFF) # MoltenVK ?
endif()

if(WIN32)
    set(LUMEN_RHI_D3D11_SUPPORTED OFF) # Not implemented yet
    set(LUMEN_RHI_D3D12_SUPPORTED OFF) # Not implemented yet
else()
    set(LUMEN_RHI_D3D11_SUPPORTED OFF)
    set(LUMEN_RHI_D3D12_SUPPORTED OFF)
endif()

if(APPLE)
    set(LUMEN_RHI_METAL_SUPPORTED OFF) # Not implemented yet
else()
    set(LUMEN_RHI_METAL_SUPPORTED OFF)
endif()

set(LUMEN_RHI_OPENGL_SUPPORTED OFF) # Not implemented yet

# Enable backend
option(LUMEN_RHI_ENABLE_VULKAN "Enable Vulkan RHI backend" ${LUMEN_RHI_VULKAN_SUPPORTED})
option(LUMEN_RHI_ENABLE_D3D11  "Enable D3D11 RHI backend"  ${LUMEN_RHI_D3D11_SUPPORTED})
option(LUMEN_RHI_ENABLE_D3D12  "Enable D3D12 RHI backend"  ${LUMEN_RHI_D3D12_SUPPORTED})
option(LUMEN_RHI_ENABLE_METAL  "Enable Metal RHI backend"  ${LUMEN_RHI_METAL_SUPPORTED})
option(LUMEN_RHI_ENABLE_OPENGL "Enable OpenGL RHI backend" ${LUMEN_RHI_OPENGL_SUPPORTED})

foreach(_backend VULKAN D3D11 D3D12 METAL)
    if (LUMEN_RHI_ENABLE_${_backend} AND NOT LUMEN_RHI_${_backend}_SUPPORTED)
        message(FATAL_ERROR "LUMEN_RHI_ENABLE_${_backend} was requested, but ${_backend} is not supported on this platform")
    endif()
endforeach()

if(NOT LUMEN_RHI_ENABLE_VULKAN 
    AND NOT LUMEN_RHI_ENABLE_D3D11 
    AND NOT LUMEN_RHI_ENABLE_D3D12
    AND NOT LUMEN_RHI_ENABLE_METAL
    AND NOT LUMEN_RHI_ENABLE_OPENGL)
    message(FATAL_ERROR "No RHI backend enabled.")
endif()

# For backends impl
add_library(engine_rhi_interface INTERFACE)
target_include_directories(engine_rhi_interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(engine_rhi_interface INTERFACE engine::core engine::io)
add_library(engine::rhi::interface ALIAS engine_rhi_interface)
# ---

add_library(engine_rhi
    src/device.cpp
)

target_include_directories(engine_rhi
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_rhi
    PUBLIC
        engine::core
        engine::io
)

target_compile_definitions(engine_rhi PUBLIC
    $<$<BOOL:${LUMEN_RHI_VULKAN_SUPPORTED}>:LUMEN_HAS_VULKAN>
    $<$<BOOL:${LUMEN_RHI_D3D11_SUPPORTED}>:LUMEN_HAS_D3D11>
    $<$<BOOL:${LUMEN_RHI_D3D12_SUPPORTED}>:LUMEN_HAS_D3D12>
    $<$<BOOL:${LUMEN_RHI_METAL_SUPPORTED}>:LUMEN_HAS_METAL>
    $<$<BOOL:${LUMEN_RHI_OPENGL_SUPPORTED}>:LUMEN_HAS_OPENGL>
)
target_compile_definitions(engine_rhi PRIVATE LUMEN_BUILD_DLL)

# Backends
if (LUMEN_RHI_ENABLE_VULKAN)
    add_subdirectory(vulkan)
    target_link_libraries(engine_rhi PRIVATE engine::rhi::vulkan)
endif()

if (LUMEN_RHI_ENABLE_D3D11)
    add_subdirectory(d3d11)
    target_link_libraries(engine_rhi PRIVATE engine::rhi::d3d11)
endif()

if (LUMEN_RHI_ENABLE_D3D12)
    add_subdirectory(d3d12)
    target_link_libraries(engine_rhi PRIVATE engine::rhi::d3d12)
endif()

if (LUMEN_RHI_ENABLE_OPENGL)
    add_subdirectory(opengl)
    target_link_libraries(engine_rhi PRIVATE engine::rhi::opengl)
endif()

add_library(engine::rhi ALIAS engine_rhi)
