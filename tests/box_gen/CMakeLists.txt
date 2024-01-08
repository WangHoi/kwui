add_executable(test_box_gen box_gen.cpp)
target_include_directories(test_box_gen PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
target_link_libraries(test_box_gen c31)
