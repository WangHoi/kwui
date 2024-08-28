kwui_add_executable(test_custom_control)
target_sources(test_custom_control PRIVATE
    custom_control.cpp
    PlotElement.cpp
    Formula.cpp
    CartesianCoord.cpp
    #    ImplicitPlot.cpp
)
if (WIN32)
    target_sources(test_custom_control PRIVATE dpi-aware.manifest)
endif ()
target_include_directories(test_custom_control PRIVATE ${CMAKE_SOURCE_DIR}/src/api)

# work around
set_target_properties(kwui PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
