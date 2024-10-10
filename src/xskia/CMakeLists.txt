list(APPEND kwui_pub_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/xskia.h)
list(APPEND kwui_priv_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/TextFlowX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TextFlowX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PainterX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PainterX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintSurfaceX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintSurfaceX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicDeviceX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GraphicDeviceX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmapX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/BitmapX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TextLayoutX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TextLayoutX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintPathX.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintPathX.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GLStateSaver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/GLStateSaver.cpp
)
if (WIN32)
    list(APPEND kwui_priv_srcs
        ${CMAKE_CURRENT_SOURCE_DIR}/GLWindowContextXWin32.cpp
    )
endif()

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)