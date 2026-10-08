find_package(spdlog CONFIG REQUIRED)

add_library(engine_core STATIC
    src/layer_stack.cpp
    src/layer.cpp

    src/events/event.cpp
    src/events/keyboard_events.cpp
    src/events/mouse_events.cpp
    src/events/window_events.cpp

    src/log.cpp
)

target_include_directories(engine_core
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_core
    PUBLIC
        engine::common
        spdlog::spdlog
)

add_library(engine::core ALIAS engine_core)
