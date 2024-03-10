list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/GraphicDevice.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Painter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TextLayout.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TextAnalysis.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TextFlow.h
)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/CustomFont.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/GraphicDevice.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Painter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextLayout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextAnalysis.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextFlow.cpp
)
set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)
