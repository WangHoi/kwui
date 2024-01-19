add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Keact.mjs.c
  COMMAND qjsc -c -m -o ${CMAKE_CURRENT_BINARY_DIR}/Keact.mjs.c ${CMAKE_CURRENT_SOURCE_DIR}/Keact.mjs
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Keact.mjs
)
add_library(kwui_jsc STATIC
	jsc.h
	jsc.cpp
	${CMAKE_CURRENT_BINARY_DIR}/Keact.mjs.c)
target_link_libraries(kwui_jsc PUBLIC absl::base absl::span)
