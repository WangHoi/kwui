add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Keact.js.c
  COMMAND qjsc -c -m -o ${CMAKE_CURRENT_BINARY_DIR}/Keact.js.c ${CMAKE_CURRENT_SOURCE_DIR}/Keact.js
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Keact.js
)
add_library(kwui_jsc STATIC
	jsc.h
	jsc.cpp
	${CMAKE_CURRENT_BINARY_DIR}/Keact.js.c)
target_link_libraries(kwui_jsc PUBLIC absl::base absl::span)
