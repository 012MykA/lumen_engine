add_library(engine_events
    src/event.cpp
    src/keyboard_events.cpp
    src/mouse_events.cpp
    src/window_events.cpp
)

target_include_directories(engine_events
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_events
    PUBLIC engine::core
)

target_compile_definitions(engine_events PRIVATE LUMEN_BUILD_DLL)

add_library(engine::events ALIAS engine_events)
