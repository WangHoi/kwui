add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Keact.js.c
  #COMMAND qjsc -c -m -o ${CMAKE_CURRENT_BINARY_DIR}/Keact.js.c ${CMAKE_CURRENT_SOURCE_DIR}/Keact.js
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../../3rdparty/qjsc_aarch64.exe -c -m -o ${CMAKE_CURRENT_BINARY_DIR}/Keact.js.c ${CMAKE_CURRENT_SOURCE_DIR}/Keact.js
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Keact.js
)
make_resource_header(vscroll_top_button.png vscroll_top_button_png)
make_resource_header(vscroll_bottom_button.png vscroll_bottom_button_png)
make_resource_header(hscroll_left_button.png hscroll_left_button_png)
make_resource_header(hscroll_right_button.png hscroll_right_button_png)
make_resource_header(icon.otf icon_otf)
add_library(kwui_resources STATIC
	"resources.h"
	"resources.cpp"
	${CMAKE_CURRENT_BINARY_DIR}/Keact.js.c
    ${CMAKE_CURRENT_BINARY_DIR}/vscroll_top_button_png.txt
    ${CMAKE_CURRENT_BINARY_DIR}/vscroll_bottom_button_png.txt
    ${CMAKE_CURRENT_BINARY_DIR}/hscroll_left_button_png.txt
    ${CMAKE_CURRENT_BINARY_DIR}/hscroll_right_button_png.txt
    ${CMAKE_CURRENT_BINARY_DIR}/icon_otf.txt)
target_include_directories(kwui_resources PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(kwui_resources PUBLIC absl::base absl::span)
