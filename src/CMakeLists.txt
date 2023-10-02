add_executable(kwui "")

add_subdirectory(base)
add_subdirectory(script)
add_subdirectory(style)
add_subdirectory(scene2d)
add_subdirectory(graph2d)

if(WIN32)
  add_subdirectory(windows)
endif()

target_sources(kwui PRIVATE main.cpp)
target_include_directories(kwui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
  kwui
  PUBLIC absl::base
         absl::span
         absl::time
         absl::strings
         absl::string_view
         absl::optional
         absl::log
         absl::log_initialize
         absl::synchronization
         absl::variant
         absl::statusor
         absl::status
         fifo_map
         quickjs)
target_link_libraries(kwui PUBLIC d2d1.lib dwrite.lib Windowscodecs.lib
                                  Imm32.lib)
target_compile_options(kwui PUBLIC /utf-8)
