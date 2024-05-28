add_subdirectory(graphics)

list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/DialogWin32.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PopupShadow.h
)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/DialogWin32.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PopupShadow.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/theme.h
  ${CMAKE_CURRENT_SOURCE_DIR}/theme.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/HiddenMsgWindow.cpp
)

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)
