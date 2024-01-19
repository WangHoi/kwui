find_program(QJSC_PROG NAMES qjsc PATHS ${CMAKE_SOURCE_DIR}/3rdparty REQUIRED)
add_custom_command(
  OUTPUT Keact.mjs.c
  COMMAND ${QJSC_PROG} -c -m -o ${CMAKE_CURRENT_BINARY_DIR}/Keact.mjs.c ${CMAKE_CURRENT_SOURCE_DIR}/Keact.mjs
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Keact.mjs
)
#[[
list(APPEND kwui_priv_srcs 
  ${CMAKE_CURRENT_BINARY_DIR}/Keact.mjs.c
)
set(kwui_priv_srcs ${kwui_priv_srcs} PARENT_SCOPE)
]]
add_library(kwui_jsc STATIC
	jsc.h
	jsc.cpp
	${CMAKE_CURRENT_BINARY_DIR}/Keact.mjs.c)
target_link_libraries(kwui_jsc PUBLIC absl::base absl::span)
