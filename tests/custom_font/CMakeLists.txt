add_executable(test_custom_font custom_font.cpp dpi-aware.manifest)
target_include_directories(test_custom_font PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(test_custom_font kwui_static)
