add_executable(native native_main.cpp dpi-aware.manifest)
target_include_directories(native PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(native kwui_static)
