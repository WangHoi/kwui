add_executable(gallery gallery_main.cpp)
target_include_directories(gallery PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(gallery kwui_static)
