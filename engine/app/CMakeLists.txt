find_package(spdlog CONFIG REQUIRED)

add_library(engine_app
    src/application.cpp
)

target_include_directories(engine_app
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_app
    PUBLIC
        engine::core
        engine::events
        engine::windowing
        engine::logging
        engine::input
)

target_compile_definitions(engine_app PRIVATE LUMEN_BUILD_DLL)

add_library(engine::app ALIAS engine_app)
