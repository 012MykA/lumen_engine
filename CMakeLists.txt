cmake_minimum_required(VERSION 3.23)
project(lumen_engine_workspace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(engine)
add_subdirectory(sandbox)
