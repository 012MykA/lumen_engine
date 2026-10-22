add_library(engine_gfx STATIC
    src/graphics_context.cpp
)

target_include_directories(engine_gfx
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_gfx
    PUBLIC
    engine::core
    engine::windowing
    engine::vulkan_core
)

add_library(engine::gfx ALIAS engine_gfx)
