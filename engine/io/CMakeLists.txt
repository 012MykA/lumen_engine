find_package(spdlog CONFIG REQUIRED)
find_package(glfw3 3.4 CONFIG REQUIRED)

add_library(engine_io
    src/log.cpp
    src/platform/glfw/glfw_input.cpp
)

target_include_directories(engine_io
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_io
    PUBLIC
        engine::core
        spdlog::spdlog
    PRIVATE
        engine::windowing
        glfw
)

target_compile_definitions(engine_io PRIVATE LUMEN_BUILD_DLL)

add_library(engine::io ALIAS engine_io)
