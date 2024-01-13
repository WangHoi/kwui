cmake_minimum_required(VERSION 3.20)

# Allow the user to specify the MSVC runtime
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif(POLICY CMP0091)

project(kwui)

include(util.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL ON)
add_subdirectory(3rdparty/abseil-cpp-20230802.1)
add_subdirectory(3rdparty/quickjspp)
add_subdirectory(3rdparty/fifo_map)

add_subdirectory(src)

add_subdirectory(examples)
add_subdirectory(tests)
