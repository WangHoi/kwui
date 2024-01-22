list(APPEND kwui_pub_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/base.h)
list(APPEND kwui_priv_srcs
  ${CMAKE_CURRENT_SOURCE_DIR}/Object.h
  ${CMAKE_CURRENT_SOURCE_DIR}/WeakObject.h
  ${CMAKE_CURRENT_SOURCE_DIR}/string_intern.h
  ${CMAKE_CURRENT_SOURCE_DIR}/scoped_setter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/log.h
  ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/PEG.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ResourceArchive.h
  ${CMAKE_CURRENT_SOURCE_DIR}/ResourceArchive.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/TaskQueue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/TaskQueue.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/lzf.h
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/lzf.cpp
  )

set(kwui_pub_srcs ${kwui_pub_srcs} PARENT_SCOPE)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)