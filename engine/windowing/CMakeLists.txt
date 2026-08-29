find_package(glfw3 3.4 CONFIG REQUIRED)

add_library(engine_windowing
    src/window_manager.cpp
    src/window.cpp
    
    src/platform/glfw/glfw_context.cpp
    src/platform/glfw/glfw_window.cpp
)

target_include_directories(engine_windowing
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_windowing
    PUBLIC  engine::core engine::events
    PRIVATE glfw
)

target_compile_definitions(engine_windowing PRIVATE LUMEN_BUILD_DLL)

add_library(engine::windowing ALIAS engine_windowing)
