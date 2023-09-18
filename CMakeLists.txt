cmake_minimum_required(VERSION 3.27)

project(quickui)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API aa1f7df0-828a-4fcd-9afc-2dc80491aca7)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rdparty/quickjspp)
add_subdirectory(src)