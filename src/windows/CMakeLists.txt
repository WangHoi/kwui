add_subdirectory(graphics)
add_subdirectory(control)

list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/Dialog.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PopupShadow.h
  ${CMAKE_CURRENT_SOURCE_DIR}/EncodingManager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/encoding_helpers.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.h
)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/Dialog.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PopupShadow.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/EncodingManager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/encoding_helpers.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/theme.h
  ${CMAKE_CURRENT_SOURCE_DIR}/theme.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ResourceManager.cpp
)

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)
