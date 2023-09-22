add_executable(kwui "")

add_subdirectory(base)
add_subdirectory(scene2d)
add_subdirectory(style)

target_sources(kwui PRIVATE
    main.cpp)
target_link_libraries(kwui PRIVATE
    quickjs
#    absl::base
#    absl::synchronization
#    absl::strings
)
# target_compile_options(kwui PRIVATE /experimental:module)
