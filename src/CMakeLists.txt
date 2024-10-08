set(kwui_priv_srcs "")
set(kwui_pub_srcs "")

add_subdirectory(api)
add_subdirectory(base)
add_subdirectory(script)
add_subdirectory(style)
add_subdirectory(scene2d)
add_subdirectory(graph2d)
add_subdirectory(resources)
if (WITH_SKIA)
    add_subdirectory(xskia)
endif ()
if (WIN32)
    add_subdirectory(windows)
endif ()
if (ANDROID)
    add_subdirectory(android)
endif ()

set(dep_libs
    absl::base
    absl::span
    absl::time
    absl::strings
    absl::string_view
    absl::optional
    absl::log
    absl::log_initialize
    absl::synchronization
    absl::variant
    absl::statusor
    absl::status
    absl::bind_front
    fifo_map
    tgaimage
    quickjs)
if (WIN32)
    list(APPEND dep_libs d3d11.lib d2d1.lib dwrite.lib Windowscodecs.lib Imm32.lib)
else ()
    list(APPEND dep_libs utf)
endif ()
if (WITH_SKIA)
    list(APPEND dep_libs skia)
endif ()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/api/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")
if (NOT ANDROID)
    install(TARGETS quickjs)
endif ()

# Build shared library
if (BUILD_SHARED_LIBS)
    add_library(kwui SHARED "")
    target_sources(kwui PUBLIC ${kwui_pub_srcs} PRIVATE ${kwui_priv_srcs})
    if (WIN32)
        target_compile_options(kwui PUBLIC /utf-8)
    endif ()
    target_compile_definitions(kwui PRIVATE -DKWUI_SHARED_LIBRARY)
    target_include_directories(kwui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(kwui PUBLIC ${dep_libs} PRIVATE kwui_resources)
    install(TARGETS kwui DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    if (WIN32)
        install(FILES $<TARGET_PDB_FILE:kwui> DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
    endif ()
else ()
    # Build static library
    add_library(c31 STATIC "")
    target_sources(c31 PUBLIC ${kwui_pub_srcs} PRIVATE ${kwui_priv_srcs})
    if (WIN32)
        target_compile_options(c31 PUBLIC /utf-8)
    endif ()
    target_compile_definitions(c31 PUBLIC -DKWUI_STATIC_LIBRARY INTERFACE -DKWUI_STATIC_LIBRARY)
    # To make msvc intellisense happy
    target_include_directories(c31 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(c31 PUBLIC ${dep_libs} PRIVATE kwui_resources)
    if (WIN32)
        bundle_static_library(c31 kwui_static)
    endif ()
endif ()

if (BUILD_TESTS)
    add_executable(kwui_utests utest_main.cpp)
    target_link_libraries(kwui_utests PRIVATE Catch2::Catch2)
    if (BUILD_SHARED_LIBS)
        target_link_libraries(kwui_utests PRIVATE kwui)
    else ()
        target_sources(kwui_utests PRIVATE c31)
    endif ()
    catch_discover_tests(kwui_utests)
endif ()
