add_library(engine_core
    src/layer_stack.cpp
    src/layer.cpp
)

target_include_directories(engine_core
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_core
    PUBLIC engine::common
)

target_compile_definitions(engine_core PRIVATE LUMEN_BUILD_DLL)

add_library(engine::core ALIAS engine_core)
