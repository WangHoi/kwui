add_executable(func_comp func_comp_main.cpp dpi-aware.manifest)
target_include_directories(func_comp PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(func_comp kwui_static)
