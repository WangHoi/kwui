add_executable(test_richtext richtext.cpp)
target_include_directories(test_richtext PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(test_richtext kwui_static)
