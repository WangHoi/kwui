list(APPEND kwui_pub_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/graph2d.h
    ${CMAKE_CURRENT_SOURCE_DIR}/Bitmap.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintBrush.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintContextInterface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintSurface.h
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintPathInterface.h)
list(APPEND kwui_priv_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/graph2d.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PaintBrush.cpp)

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)
