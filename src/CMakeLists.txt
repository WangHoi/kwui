add_executable(quickui "")
target_sources(quickui PRIVATE
    main.cpp)
target_link_libraries(quickui PRIVATE
    quickjs)
