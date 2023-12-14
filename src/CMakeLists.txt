set(kwui_priv_srcs "")
set(kwui_pub_srcs "")

add_subdirectory(api)
add_subdirectory(base)
add_subdirectory(script)
add_subdirectory(style)
add_subdirectory(scene2d)
add_subdirectory(graph2d)

if(WIN32)
  add_subdirectory(windows)
endif()

set(dep_libs
    absl::base
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
    absl::bind_front
    fifo_map
    quickjs
    d2d1.lib dwrite.lib Windowscodecs.lib
    Imm32.lib)

# Build shared library
#[[
add_library(kwui SHARED "")
target_sources(kwui PUBLIC ${kwui_pub_srcs} PRIVATE ${kwui_priv_srcs})
target_compile_options(kwui PUBLIC /utf-8)
target_compile_definitions(kwui PRIVATE -DKWUI_SHARED_LIBRARY)
target_include_directories(kwui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(kwui PUBLIC ${dep_libs})
]]

# Build static library
add_library(c31 STATIC "")
target_sources(c31 PUBLIC ${kwui_pub_srcs} PRIVATE ${kwui_priv_srcs})
target_compile_options(c31 PUBLIC /utf-8)
target_compile_definitions(c31 PUBLIC -DKWUI_STATIC_LIBRARY)
target_include_directories(c31 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(c31 PUBLIC ${dep_libs})
