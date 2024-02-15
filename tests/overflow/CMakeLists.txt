add_executable(test_overflow overflow.cpp dpi-aware.manifest)
target_include_directories(test_overflow PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(test_overflow kwui_static)
