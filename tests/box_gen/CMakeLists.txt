add_executable(test_box_gen box_gen.cpp dpi-aware.manifest)
target_include_directories(test_box_gen PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(test_box_gen kwui_static)
