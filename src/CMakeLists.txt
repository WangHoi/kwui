add_executable(kwui "")

add_subdirectory(base)
add_subdirectory(script)
add_subdirectory(style)
add_subdirectory(scene2d)

target_sources(kwui PRIVATE
    main.cpp)
target_link_libraries(kwui PRIVATE
    quickjs
#    absl::base
#    absl::synchronization
#    absl::strings
    fifo_map
)
target_compile_options(kwui PRIVATE /utf-8)
