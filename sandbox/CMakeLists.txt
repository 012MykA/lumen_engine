cmake_minimum_required(VERSION 3.23)
project(sandbox)

add_executable(${PROJECT_NAME}
    src/sandbox_app.cpp
    src/vulkan_render_layer.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE lumen::engine)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
endif()
