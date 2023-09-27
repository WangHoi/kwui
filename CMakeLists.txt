cmake_minimum_required(VERSION 3.20)

project(kwui)

set(CMAKE_CXX_STANDARD 17)

set(ABSL_PROPAGATE_CXX_STD ON)
add_subdirectory(3rdparty/abseil-cpp-20230802.1)
add_subdirectory(3rdparty/quickjspp)
add_subdirectory(3rdparty/fifo_map)

add_subdirectory(src)
