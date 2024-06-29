add_library(quickjs STATIC "")
target_compile_definitions(quickjs PUBLIC
  JS_STRICT_NAN_BOXING # this option enables x64 build on Windows/MSVC
  CONFIG_JSX)

file(READ "VERSION.txt" VERSION)
file(CONFIGURE OUTPUT "quickjs-version.h" CONTENT "#define QUICKJS_VERSION \"${VERSION}\"")
target_sources(quickjs PRIVATE
"cutils.h"
"cutils.c"
"libregexp.c"
"libunicode.c"
"quickjs.c"
"quickjs-libc.c"
"libbf.c"
"libregexp.h"
"libregexp-opcode.h"
"libunicode.h"
"libunicode-table.h"
"list.h"
"quickjs.h"
"quickjs-atom.h"
"quickjs-libc.h"
"quickjs-opcode.h"
"quickjs-jsx.h"
"quickjs-version.h"
)
target_include_directories(quickjs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(qjsc qjsc.c)
target_link_libraries(qjsc quickjs)