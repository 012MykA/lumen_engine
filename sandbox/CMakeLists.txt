cmake_minimum_required(VERSION 3.23)
project(sandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}
    src/sandbox_app.cpp
    src/example_layer.cpp
    src/platform.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE lumen_engine)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
