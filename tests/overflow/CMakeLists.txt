kwui_add_executable(test_overflow)
target_sources(test_overflow PRIVATE overflow.cpp)
if(WIN32)
    target_sources(test_overflow PRIVATE dpi-aware.manifest)
endif()
target_include_directories(test_overflow PRIVATE ${CMAKE_SOURCE_DIR}/src/api)
