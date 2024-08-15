list(APPEND kwui_pub_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/kwui_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/kwui_main.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/Application.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptEngine.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptValue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/CustomElement.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/CustomElementPaint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui_capi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/Application_capi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptEngine_capi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptValue_capi.h
)
list(APPEND kwui_priv_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptValue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/CustomElementPaint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/Application_capi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptEngine_capi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/kwui/ScriptValue_capi.cpp
)

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)