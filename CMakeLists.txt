cmake_minimum_required(VERSION 3.23)
project(lumen-engine LANGUAGES CXX)


add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp
)

target_compile_features(${CMAKE_PROJECT_NAME}
    PRIVATE
        cxx_std_23
)
