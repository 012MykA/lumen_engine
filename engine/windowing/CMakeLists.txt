find_package(glfw3 3.4 CONFIG REQUIRED)

add_library(engine_windowing STATIC
    src/window_manager.cpp
    src/window.cpp
    
    src/platform/glfw/glfw_window_context.cpp
    src/platform/glfw/glfw_window_driver.cpp
)

target_include_directories(engine_windowing
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_windowing
    PUBLIC
        engine::core
        glfw
)

add_library(engine::windowing ALIAS engine_windowing)
