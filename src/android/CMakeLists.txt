list(APPEND kwui_pub_srcs
  )
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/DialogAndroid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/DialogAndroid.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/SurfaceAndroid.h
  ${CMAKE_CURRENT_SOURCE_DIR}/SurfaceAndroid.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui_jni.h
  ${CMAKE_CURRENT_SOURCE_DIR}/kwui_jni.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Application_jni.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Application_jni.cpp
)
set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)
