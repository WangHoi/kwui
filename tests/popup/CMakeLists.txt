add_executable(test_popup popup.cpp dpi-aware.manifest)
target_include_directories(test_popup PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(test_popup kwui_static)
