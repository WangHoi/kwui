list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/script.h
)
list(APPEND kwui_priv_srcs 
  ${CMAKE_CURRENT_SOURCE_DIR}/script.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ComponentState.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ComponentState.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ContextId.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ContextId.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ElementRef.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ElementRef.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/EventPort.h
  ${CMAKE_CURRENT_SOURCE_DIR}/EventPort.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Value.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Value.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Keact.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Keact.cpp
)
set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)