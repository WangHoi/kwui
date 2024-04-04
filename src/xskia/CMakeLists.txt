list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/xskia.h)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/TextFlowX.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TextFlowX.cpp
  )

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)