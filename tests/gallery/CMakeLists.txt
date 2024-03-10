add_executable(gallery gallery_main.cpp dpi-aware.manifest)
target_include_directories(gallery PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(gallery kwui_static)
