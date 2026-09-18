find_package(spdlog CONFIG REQUIRED)

add_library(engine_logging
    src/log.cpp
)

target_include_directories(engine_logging
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(engine_logging
    PUBLIC
        engine::core
        spdlog::spdlog
)

target_compile_definitions(engine_logging PRIVATE LUMEN_BUILD_DLL)

add_library(engine::logging ALIAS engine_logging)
