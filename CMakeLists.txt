cmake_minimum_required(VERSION 3.26)

project(kwui)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API aa1f7df0-828a-4fcd-9afc-2dc80491aca7)
set(CMAKE_CXX_STANDARD 20)

# set(ABSL_PROPAGATE_CXX_STD ON)
# add_subdirectory(3rdparty/abseil-cpp-20230802.1)
add_subdirectory(3rdparty/quickjspp)
add_subdirectory(3rdparty/PEGTL)
add_subdirectory(3rdparty/fifo_map)

add_subdirectory(src)