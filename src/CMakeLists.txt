add_executable(quickui "")

add_subdirectory(base)
add_subdirectory(scene2d)

target_sources(quickui PRIVATE
    main.cpp)
target_link_libraries(quickui PRIVATE
    quickjs
#    absl::base
#    absl::synchronization
#    absl::strings
)
target_compile_options(quickui PRIVATE /experimental:module)
