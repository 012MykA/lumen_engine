cmake_minimum_required(VERSION 3.23)
project(sandbox)

add_executable(${PROJECT_NAME}
    src/SandboxApp.cpp
    src/ExampleLayer.cpp
)

target_link_libraries(${PROJECT_NAME}
    lumen_engine
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

if (WIN32 AND BUILD_SHARED_LIBS)
    get_target_property(ENGINE_TYPE lumen_engine TYPE)
    if (ENGINE_TYPE STREQUAL "SHARED_LIBRARY")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:lumen_engine>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()
endif()
