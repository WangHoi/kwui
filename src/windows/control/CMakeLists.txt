list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/control.h)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/LineEditControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LineEditControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ProgressBarControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ProgressBarControl.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SpinnerControl.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SpinnerControl.cpp
)
set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)