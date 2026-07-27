cmake_minimum_required(VERSION 3.23)
project(sandbox)

add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${CMAKE_PROJECT_NAME}
    lumen_engine
)

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE cxx_std_23)

if (WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:lumen_engine>
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>
    )
endif()
