list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/scene2d.h
)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/geom_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Node.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Node.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Scene.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Scene.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Event.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Attribute.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Control.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Control.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/KmlControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/KmlControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ButtonControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ButtonControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ImageControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ImageControl.cpp
#  ${CMAKE_CURRENT_SOURCE_DIR}/ScrollbarControl.h
#  ${CMAKE_CURRENT_SOURCE_DIR}/ScrollbarControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ProgressBarControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ProgressBarControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SpinnerControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SpinnerControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/LineEditControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LineEditControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/NativeViewControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/NativeViewControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextLayout.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TextLayout.cpp
)
set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)