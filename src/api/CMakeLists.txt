list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui/kwui_export.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui/Application.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptEngine.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptValue.h
)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui/Application.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptEngine.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptValue.cpp
)

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)