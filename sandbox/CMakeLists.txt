cmake_minimum_required(VERSION 3.23)
project(sandbox)

find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME}
    src/sandbox_app.cpp
    src/vulkan_renderer_layer.cpp
)

target_link_libraries(${PROJECT_NAME}
    lumen::engine
    Vulkan::Vulkan
)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
    )
endif()
