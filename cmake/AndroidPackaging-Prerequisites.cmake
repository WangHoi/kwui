
# Hint: `ANDROID_HOME` 
# Hint: `JAVA_HOME` = `C:/Program Files/Android/Android Studio/jre` on windows

# @defines
#   "CMAKE_SYSTEM_NAME": "Android",
#   "CMAKE_SYSTEM_VERSION": "29",
#   "CMAKE_ANDROID_STL_TYPE": "c++_static",
#   "CMAKE_ANDROID_EXCEPTIONS": "TRUE",
#   "CMAKE_ANDROID_ARCH_ABI": "arm64-v8a" ....OR.... "armeabi-v7a",# 
# @user-defines
#   "CMAKE_ANDROID_SDK": "$env{ANDROID_HOME}",
#   "CMAKE_ANDROID_NDK": "$env{ANDROID_HOME}/ndk/25.1.8937393",
#   "JAVA_HOME": "$env{JAVA_HOME}"

if ( NOT CMAKE_SYSTEM_NAME STREQUAL "Android")
    return()
endif()


message(CHECK_START "AndroidPackaging-Prerequisites")
list(APPEND CMAKE_MESSAGE_INDENT "  ")
unset(MISSING_ANDROID_COMPONENTS)


if ( NOT DEFINED ANDROID_HOME AND DEFINED ENV{ANDROID_HOME}) 
    set(ANDROID_HOME $ENV{ANDROID_HOME} )
endif()

if ( NOT DEFINED CMAKE_ANDROID_SDK AND DEFINED ANDROID_HOME) 
    message( STATUS  "Setting CMAKE_ANDROID_SDK from ENV{ANDROID_HOME}" )
	file(TO_CMAKE_PATH "${ANDROID_HOME}" CMAKE_ANDROID_SDK)
endif()

message(CHECK_START "Find Android SDK")
if( DEFINED CMAKE_ANDROID_SDK AND EXISTS "${CMAKE_ANDROID_SDK}")
    message( CHECK_PASS "CMAKE_ANDROID_SDK is ${CMAKE_ANDROID_SDK}" )
else()
    if( NOT DEFINED CMAKE_ANDROID_SDK )
        message( CHECK_FAIL "CMAKE_ANDROID_SDK was not defined.\n"
                            "Get Android SDK from Studio or Commandline-Tools:\n"
                            "https://developer.android.com/studio#downloads" )
    else()
        message( CHECK_FAIL "CMAKE_ANDROID_SDK '${CMAKE_ANDROID_SDK}' is not a valid directory." )
    endif()
    list(APPEND MISSING_ANDROID_COMPONENTS SDK)
endif()

message(CHECK_START "Find Android NDK")
if ( NOT DEFINED ANDROID_NDK_HOME AND DEFINED ENV{ANDROID_NDK_HOME}) 
    set(ANDROID_NDK_HOME $ENV{ANDROID_NDK_HOME} )
endif()

if ( NOT DEFINED CMAKE_ANDROID_NDK AND DEFINED ANDROID_NDK_HOME) 
    message( STATUS  "Setting CMAKE_ANDROID_NDK from ENV{ANDROID_NDK_HOME}" )
    file(TO_CMAKE_PATH "${ANDROID_NDK_HOME}" CMAKE_ANDROID_NDK)
endif()

if( DEFINED CMAKE_ANDROID_NDK AND EXISTS "${CMAKE_ANDROID_NDK}")
    message( CHECK_PASS "CMAKE_ANDROID_NDK is '${CMAKE_ANDROID_NDK}'" )
else()
    if( NOT DEFINED CMAKE_ANDROID_NDK )
        message( CHECK_FAIL "CMAKE_ANDROID_NDK was not defined." )
    else()
        message( CHECK_FAIL "CMAKE_ANDROID_NDK '${CMAKE_ANDROID_NDK}' is not a valid directory." )
    endif()
    list(APPEND MISSING_ANDROID_COMPONENTS NDK)
endif()

message(CHECK_START "Finding JAVA home")
# TODO: Document this behaviour....
# org.gradle.java.home=@JAVA_HOME@
find_package(Java 11 REQUIRED)
if ( NOT JAVA_HOME ) # Extract `JAVA_HOME` from parent path of `Java_JAVA_EXECUTABLE`
    cmake_path(GET Java_JAVA_EXECUTABLE PARENT_PATH  JAVA_HOME)
    cmake_path(GET JAVA_HOME PARENT_PATH JAVA_HOME)
endif()    
if( NOT JAVA_HOME )
    message( CHECK_FAIL "JAVA_HOME needs to be defined." )
    list(APPEND MISSING_ANDROID_COMPONENTS JAVA)
else()
    message( CHECK_PASS "JAVA_HOME is ${JAVA_HOME}" )
endif()    

list(POP_BACK CMAKE_MESSAGE_INDENT)
if(MISSING_ANDROID_COMPONENTS)
    message(CHECK_FAIL "Missing component(s): ${MISSING_ANDROID_COMPONENTS}")
else()
    message(CHECK_PASS "All components found")
endif()
