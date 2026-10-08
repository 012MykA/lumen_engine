add_library(engine_runtime STATIC
    src/application.cpp
)

target_include_directories(engine_runtime
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_runtime
    PUBLIC
        engine::core
        engine::windowing
)

add_library(engine::runtime ALIAS engine_runtime)
