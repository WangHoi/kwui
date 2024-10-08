cmake_minimum_required(VERSION 3.20)

# Allow the user to specify the MSVC runtime
if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif(POLICY CMP0091)

project(kwui)

include(FeatureSummary)

option(CRT_STATIC "Link with static MSVC runtime" OFF)
add_feature_info(CRT_STATIC CRT_STATIC "Link with static MSVC runtime")
option(BUILD_SHARED_LIBS "Build shared kwui library" ON)
add_feature_info(BUILD_SHARED_LIBS BUILD_SHARED_LIBS "Build shared kwui library")
option(BUILD_TESTS "Build kwui tests" ON)
add_feature_info(BUILD_TESTS BUILD_TESTS "Build kwui tests")
if(WIN32)
option(WITH_SKIA "Build with Skia renderer" OFF)
else()
option(WITH_SKIA "Build with Skia renderer" ON)
endif()
add_feature_info(WITH_SKIA WITH_SKIA "Build with Skia renderer")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    feature_summary(WHAT ENABLED_FEATURES DISABLED_FEATURES PACKAGES_FOUND)
    feature_summary(FILENAME ${CMAKE_CURRENT_BINARY_DIR}/features.log WHAT ALL)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_GENERATOR "Ninja")

include(util.cmake)
include(cmake/AndroidPackaging-Prerequisites.cmake)
include(cmake/AndroidPackaging.cmake)

if(BUILD_SHARED_LIBS)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if (WIN32)

# workaround std::mutex crash on VS2022 17.10 STL
add_compile_definitions("$<$<COMPILE_LANGUAGE:C,CXX>:_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR=1>")

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
	# For memory leak detection (see https://github.com/MicrosoftDocs/visualstudio-docs/blob/master/docs/debugger/finding-memory-leaks-using-the-crt-library.md )
	# add_definitions(-D "_CRTDBG_MAP_ALLOC")

	# Used debug build options:
	# - "Debug Information Format"	= "Program Database for Edit And Continue (/ZI)" ( "/Z7, /Zi, /ZI (Debug Information Format)"-documentation at https://msdn.microsoft.com/en-us/library/958x11bc.aspx )
	# - "Basic Runtime Checks"		= "Default" instead of "/RTC1" which is causing a massive slow-down when debugging
	unrimp_replace_cmake_cxx_flags(/Zi /ZI)
	unrimp_replace_cmake_cxx_flags(/RTC1 "")

	# TODO(co) "Just My Code" = "/JMC" (see https://blogs.msdn.microsoft.com/vcblog/2018/06/29/announcing-jmc-stepping-in-visual-studio/ ) is currently disabled due to horrible observed performance
	#          impact in Visual Studio 2017 15.8.1: In my scenario: 13.3 ms per frame without JMC and 21.5 ms per frame with JMC (major hotspot is "__CheckForDebuggerJustMyCode")
	# Used options:
	# - "Just My Code" = "/JMC" (see https://blogs.msdn.microsoft.com/vcblog/2018/06/29/announcing-jmc-stepping-in-visual-studio/ )
	# set(CompilerFlags "/JMC")
	# set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${CompilerFlags}")
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CompilerFlags}")
else()
	# Used release build options:
	# - "Enable Intrinsic Functions"		= "Yes (/Oi)"
	# - "Favor Size Or Speed"				= "Favor fast code (/Ot)"
	# - "Omit Fame Pointers"				= "Yes (/Oy)"
	# - "Enable String Pooling"				= "Yes (/GF)"
	# - "Security Check"					= "Disable Security Check (/GS-)"
	# - "Enable Function-Level Linking"		= "Yes (/Gy)"
	# - "Floating Point Model"				= "Fast (/fp:fast)"
	# - "Enable Floating Point Exceptions"	= "No (/fp:except-)"
	# - "Whole Program Optimization"		= "Yes (/GL)" with linker flag "LTCG"
	# - "Gw" - see https://blogs.msdn.microsoft.com/vcblog/2013/09/11/introducing-gw-compiler-switch/
	# - "Enable Large Addresses"			= "Yes (/LARGEADDRESSAWARE)"
	# - "Enable COMDAT Folding"				= "Yes (/OPT:ICF)"
	# - "References"						= "Yes (/OPT:REF)"
	set(CompilerFlags "-Oi -Ot -Oy -GF -GS- -Gy -fp:fast -fp:except- -GL /Gw /EHsc")
	if(${CMAKE_GENERATOR} MATCHES "Ninja")	# Ninja is used for "CMake support in Visual" - https://blogs.msdn.microsoft.com/vcblog/2016/10/05/cmake-support-in-visual-studio/
		set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${CompilerFlags}")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CompilerFlags}")
		# set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE /OPT:ICF /OPT:REF")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE /OPT:ICF /OPT:REF")
	else()
		set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS} ${CompilerFlags}")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CompilerFlags}")
		# set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
		set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE /OPT:ICF /OPT:REF")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE /OPT:ICF /OPT:REF")
	endif()
endif()
endif()

set(CMAKE_CXX_STANDARD 17)

if(CRT_STATIC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL OFF)

set(BUILD_SHARED_LIBS_SAVED "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(3rdparty/abseil-cpp-20230802.1)
set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_SAVED}")

add_subdirectory(3rdparty/quickjspp)
add_subdirectory(3rdparty/fifo_map)
add_subdirectory(3rdparty/tgaimage)
if(WITH_SKIA)
add_compile_definitions(WITH_SKIA=1)
add_subdirectory(3rdparty/skia)
endif()
add_subdirectory(3rdparty/utf)

set(BUILD_SHARED_LIBS_SAVED "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(3rdparty/Catch2-3.6.0)
set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_SAVED}")
list(APPEND CMAKE_MODULE_PATH 3rdparty/Catch2-3.6.0/extras)
include(CTest)
include(Catch)

add_subdirectory(src)

if(BUILD_TESTS)
add_subdirectory(tests)
endif()