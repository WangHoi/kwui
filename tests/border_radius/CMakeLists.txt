kwui_add_executable(test_border_radius)
target_sources(test_border_radius PRIVATE border_radius.cpp)
if(WIN32)
  target_sources(test_border_radius PRIVATE dpi-aware.manifest)
endif()
target_include_directories(test_border_radius PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
