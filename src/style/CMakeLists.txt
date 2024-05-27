list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/style.h
)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleFont.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleValue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleValue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleClass.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StylePseudoClass.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleRule.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleRule.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleColor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StyleColor.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Layout.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Layout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/BlockLayout.h
  ${CMAKE_CURRENT_SOURCE_DIR}/BlockLayout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/InlineLayout.h
  ${CMAKE_CURRENT_SOURCE_DIR}/InlineLayout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TextFlow.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TextFlow.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/InlineBlockLayout.h
  ${CMAKE_CURRENT_SOURCE_DIR}/InlineBlockLayout.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/StylePaint.h
  ${CMAKE_CURRENT_SOURCE_DIR}/StylePaint.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/BoxConstraintSolver.h
  ${CMAKE_CURRENT_SOURCE_DIR}/BoxConstraintSolver.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/LayoutObject.h
  ${CMAKE_CURRENT_SOURCE_DIR}/LayoutObject.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ScrollObject.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ScrollObject.cpp)

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)