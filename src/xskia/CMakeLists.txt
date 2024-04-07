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
  )

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)