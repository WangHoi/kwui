
option( ANDRODIPACKAGING_GRADLE_DAEMON 
    "Use Gradle Daemon can be used for faster builds. WARNING: May hang builds for integrated IDE (e.g. MSVC 17.3.5)" 
    FALSE )

set(GRADLE_USER_HOME "${PROJECT_BINARY_DIR}/.gradle/" CACHE INTERNAL "Gradle cache")
 
function(make_apk_ndk_library TARGET)
    # Global variables
    set(ANDROID_ABIS ${CMAKE_ANDROID_ARCH_ABI})
    set(ANDROID_SDK ${CMAKE_ANDROID_SDK})
    set(ANDROID_NDK ${CMAKE_ANDROID_NDK})

    cmake_parse_arguments(
        ""
        "" # Options
        "" #one value keywords
        "" # Multi-value keywords
        ${ARGN}
    )

    # Default template configuration file list
    if ( NOT DEFINED ANDROIDPACKAGING_TEMPLATE_PATH )
        set(ANDROIDPACKAGING_TEMPLATE_PATH ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/AndroidPackaging.template/ )
    endif()
    if ( NOT EXISTS ${ANDROIDPACKAGING_TEMPLATE_PATH})
        message(FATAL_ERROR  "Missing Android Package-Template `${ANDROIDPACKAGING_TEMPLATE_PATH}`")
    endif()

    # Default template configuration file list
    if ( NOT DEFINED ANDROIDPACKAGING_CONFIGFILE )
        set(ANDROIDPACKAGING_CONFIGFILE ${ANDROIDPACKAGING_TEMPLATE_PATH}/.cmakeconfigure )
    endif()    
    if ( NOT EXISTS ${ANDROIDPACKAGING_CONFIGFILE})
        message(FATAL_ERROR  "Missing Config-File `${ANDROIDPACKAGING_CONFIGFILE}`")
    endif()

    add_custom_target(${TARGET}.APK ALL)

    set( APK.BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_apk")
    set_target_properties( ${TARGET} 
        PROPERTIES
            APK.BUILD_ROOT ${APK.BUILD_ROOT} )

    file(MAKE_DIRECTORY ${APK.BUILD_ROOT})
    
    # TODO: Look at Object library instead for android::native_app_glue? https://cmake.org/cmake/help/latest/command/add_library.html#object-libraries 
    # .. or less so `--whole-archive`
    # Pretend ANativeActivity_onCreate is undefined, to force linking of library modules that define it i.e. App-glue
    set_target_properties( ${TARGET} PROPERTIES LINK_FLAGS  "-u ANativeActivity_onCreate" )

    # TODO: This shoudl replace above....
    # Export ANativeActivity_onCreate(),
    # Refer to: https://github.com/android-ndk/ndk/issues/381.
    #set(CMAKE_SHARED_LINKER_FLAGS
    #     "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

    set(ANDROIDPACKAGING_LIB_ROOT "${APK.BUILD_ROOT}/src/main/jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
    file(MAKE_DIRECTORY ${ANDROIDPACKAGING_LIB_ROOT})

    set(ANDROIDPACKAGING_ROOT_NS "prj.kwui") 
    
    if ( NOT DEFINED ANDROID_VERSIONCODE)
        # Android uses an integer version-code to determine upgrade vs downgrade operations
        # (MAJOR * 50000000) + (MINOR * 1000000) + (PATCH * 10000) + COMMIT"
        # NOTE: Max 2100000000 i.e. MAJOR<42, MINOR<50, PATCH<100, COMMIT<10000
        if ( VERSION_COMMIT GREATER_EQUAL 10000 )
            message(WARNING "VERSION_PATCH ${VERSION_COMMIT} which exceeds the 10000")
        endif()
        if ( VERSION_COMMIT GREATER_EQUAL 10000 )
            message(WARNING "VERSION_PATCH ${VERSION_PATCH} which exceeds the 100")
        endif()
        math(EXPR ANDROID_VERSIONCODE "((${VERSION_MAJOR}+0) * 50000000) + ((${VERSION_MINOR}+0) * 1000000) + ((${VERSION_PATCH}+0) * 10000) + ((${VERSION_COMMIT}+0) * 1)")
    endif()

    if ( ANDROID_VERSIONCODE GREATER_EQUAL 2100000000 )
        message(WARNING "Android 'application:versionCode' is ${ANDROID_VERSIONCODE} which exceeds the 2100000000 limit")
    endif()

    if ( NOT DEFINED ANDROID_VERSIONNAME)
        set(ANDROID_VERSIONNAME "${VERSION_FULL}" )
    endif()    
    
    # Package==Application for now
    set(ANDROID_APPLICATION_NAME ${TARGET})  
    set(ANDROID_APPLICATION_ID ${ANDROIDPACKAGING_ROOT_NS}.${ANDROID_APPLICATION_NAME})
    set(ANDROID_APPLICATION_VERSIONNAME "${ANDROID_VERSIONNAME}")
    set(ANDROID_APPLICATION_VERSIONCODE "${ANDROID_VERSIONCODE}") 

    # Package==Application for now
    # - need not be @see #'https://code.luasoftware.com/tutorials/android/android-studio-rename-package'
    set(ANDROID_PACKAGE_NAME ${TARGET})  
    string(REPLACE "-" "_" ANDROID_PACKAGE_NAME ${ANDROID_PACKAGE_NAME}) # Sanitize package-id as Android package name cannot contain '-'
    
    set(ANDROID_PACKAGE_ID "${ANDROIDPACKAGING_ROOT_NS}.${ANDROID_PACKAGE_NAME}")
    set(ANDROID_PACKAGE_VERSIONNAME "${ANDROID_VERSIONNAME}") 
    set(ANDROID_PACKAGE_VERSIONCODE ${ANDROID_VERSIONCODE}) 

    #message( "ANDROID_PACKAGE_NAME ${ANDROID_PACKAGE_NAME}")
    #message( "ANDROID_PACKAGE_ID ${ANDROID_PACKAGE_ID}")

    set(ANDROIDPACKAGING_SOURCES)
    set(ANDROID_ADDITIONAL_PARAMS)
    string(REPLACE "\",\"" " " ANDROID_ABIS_SPACES "${ANDROID_ABIS}")
    
    if (CMAKE_BUILD_TYPE STREQUAL Release)
        set(SOURCEEXCLUDE_DEBUGPREFIX  "!") # Exclude debug files
    else()    
        set(ANDROID_ADDITIONAL_PARAMS "${ANDROID_ADDITIONAL_PARAMS} android:debuggable=\"true\"") #@todo  android:extractNativeLibs=\"true\"
        set(SOURCEEXCLUDE_DEBUGPREFIX  "") # Include debug files
    endif()

    # Load list of files in `android_project_template`
    # * ${TARGET}.APK.SOURCES - List of files that need `configure_file` if suffic is `.in` otherwise just copied
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${ANDROIDPACKAGING_CONFIGFILE})
    file(STRINGS ${ANDROIDPACKAGING_CONFIGFILE} ANDROIDPACKAGING_CONFIGFILE REGEX "^[^#${SOURCEEXCLUDE_DEBUGPREFIX}](.*)")
    #list(TRANSFORM ANDROIDPACKAGING_CONFIGFILE PREPEND ${ANDROIDPACKAGING_TEMPLATE_PATH}/)
    apk_target_sources( ${TARGET}
        BASE_DIRECTORY ${ANDROIDPACKAGING_TEMPLATE_PATH}/
        ${ANDROIDPACKAGING_CONFIGFILE} 
    )

    #file(GENERATE OUTPUT "~/test.txt" CONTENT "TARGET_RUNTIME_DLLS: $<TARGET_RUNTIME_DLLS:${TARGET}>")
    if(true) #TODO: Obselete as of cmake 3.21 provides $<TARGET_RUNTIME_DLLS:${TARGET}>
        # Get all 'dependies' of the TARGET
        get_target_property( JNILIB_TARGETS ${TARGET} LINK_LIBRARIES  )
        list(APPEND JNILIB_TARGETS ${TARGET}) # Add self to JNILIB list

        # Copy shared libaries into `ANDROIDPACKAGING_LIB_ROOT`
        foreach( TARGET ${JNILIB_TARGETS})
            if ( TARGET ${TARGET}  )
                get_target_property( type ${TARGET} TYPE )
                if ( type STREQUAL "SHARED_LIBRARY" )
                    add_custom_command( TARGET ${TARGET}.APK PRE_BUILD
                        DEPENDS ${TARGET}
                        COMMENT "Collecting ${TARGET}"
                        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET}> ${ANDROIDPACKAGING_LIB_ROOT} )
                endif()
            endif()
        endforeach()

        #Copy C++ runtime into `ANDROIDPACKAGING_LIB_ROOT` if using shared
        if( CMAKE_ANDROID_STL_TYPE  MATCHES "_shared$" )
            message( "CMAKE_ANDROID_STL_TYPE ${CMAKE_ANDROID_STL_TYPE} " )
            find_library(ANDROIDPACKAGING_STLRUNTIME_PATH ${CMAKE_ANDROID_STL_TYPE} REQUIRED)
            message( "ANDROIDPACKAGING_STLRUNTIME_PATH ${ANDROIDPACKAGING_STLRUNTIME_PATH}" )
            message( "ANDROIDPACKAGING_LIB_ROOT ${ANDROIDPACKAGING_LIB_ROOT}" )
            add_custom_command( TARGET ${TARGET}.APK PRE_BUILD
                COMMENT "Collecting shared runtime ${ANDROIDPACKAGING_STLRUNTIME_PATH}"
                COMMAND ${CMAKE_COMMAND} -E copy "${ANDROIDPACKAGING_STLRUNTIME_PATH}" "${ANDROIDPACKAGING_LIB_ROOT}" )
        endif()

    else() #TODO: Doens't appear to work? need dependencies flaggedas imports somehow...?
        add_custom_command( TARGET ${TARGET}.APK PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy
                            $<TARGET_RUNTIME_DLLS:${TARGET}>
                            ${ANDROIDPACKAGING_LIB_ROOT}
                        COMMAND_EXPAND_LISTS)
    endif()

    if ( NOT ANDRODIPACKAGING_GRADLE_DAEMON )
        set( GRADLE_OPTS_DAEMON --no-daemon)
    endif()
#            --gradle-user-home ${GRADLE_USER_HOME}
    add_custom_command(
        TARGET ${TARGET}.APK
        #OUTPUT ${APK.BUILD_ROOT}/${hostSystemName}-build/
        COMMENT "Gathering gradle dependencies"
        COMMAND ./gradlew $<IF:$<CONFIG:Debug>,assembleDebug,assembleRelease>
            ${GRADLE_OPTS_DAEMON}
            --no-rebuild 
        WORKING_DIRECTORY ${APK.BUILD_ROOT}
        # Ninja Delete Bug on windows: BYPRODUCTS        ${APK.BUILD_ROOT}/${hostSystemName}-build/
        DEPENDS $<TARGET_PROPERTY:${TARGET},APK.SOURCES>
        VERBATIM
        USES_TERMINAL
    )

    set(ANDROID_ADB "${ANDROID_SDK}/platform-tools/adb")
    if(false) # INSTALL?
        add_custom_command(
            TARGET ${TARGET}.APK POST_BUILD
                COMMAND
                    ${ANDROID_ADB}
                    install -r "build/outputs/apk/debug/${TARGET}_apk-debug.apk"
                WORKING_DIRECTORY "${APK.BUILD_ROOT}"
                VERBATIM
                USES_TERMINAL
            )
        add_custom_command(
            TARGET ${TARGET}.APK POST_BUILD
                COMMAND
                    ${ANDROID_ADB} 
                    shell monkey -p ${ANDROID_APPLICATION_ID}.${ANDROID_TARGET_NAME}.debug -c android.intent.category.LAUNCHER 1
                WORKING_DIRECTORY "${APK.BUILD_ROOT}"
                VERBATIM
                USES_TERMINAL
            )
            
    endif()

    set_directory_properties( PROPERTIES  ADDITIONAL_MAKE_CLEAN_FILES
        ${APK.BUILD_ROOT}/build 
    )

endfunction()


function(apk_target_sources TARGET )
    cmake_parse_arguments(
        ""
        "" # Options
        "BASE_DIRECTORY" #one value keywords
        "SOURCES" # Multi-value keywords
        ${ARGN}
    )

    get_target_property( APK.SOURCES ${TARGET} APK.SOURCES )
    get_target_property( APK.BUILD_ROOT ${TARGET} APK.BUILD_ROOT )
    if ( NOT APK.SOURCES )
        set(APK.SOURCES)
    endif()

    set( INPUT.SOURCES ${_UNPARSED_ARGUMENTS} )
    list(APPEND INPUT.SOURCES ${SOURCES} )
    list(TRANSFORM INPUT.SOURCES REPLACE "^!" "" ) # Remove '!' debug prefix'

   foreach(source ${INPUT.SOURCES})
        set( output ${APK.BUILD_ROOT}/${source} )
        set( input ${_BASE_DIRECTORY}/${source} )

        if ( EXISTS ${input} )
            message("${source} copy.")
            configure_file(${input} ${output} COPYONLY)
        else() # File may have '.in' postfix 
            set(input ${_BASE_DIRECTORY}/${source}.in)
            message("${source} configure.")
            configure_file(${input} ${output} @ONLY)
        endif()

        list( APPEND APK.SOURCES ${output}) 

    endforeach()

    #message( "${TARGET}::APK.SOURCES = ${APK.SOURCES}")
    
    set_property( 
        TARGET ${TARGET} 
        PROPERTY APK.SOURCES ${APK.SOURCES} )
endfunction()

function(apk_add_executable TARGET)
    add_library( ${TARGET} SHARED )
    make_apk_ndk_library( ${TARGET} ${ARGN} )

    # We implcitly add common link targets
    target_link_libraries( ${TARGET} 
        $<$<BOOL:${ANDROID}>:android> # @note libandroid.so
        $<$<BOOL:${ANDROID}>:log>
    )
endfunction()

# TODO: good idea or not!? We still want to make normal executables for command-line tests etc!
# TODO: Support a `APK` property like the `WIN32` one already supported
# function(add_executable name)
#     apk_add_executable(${name} ${ARGN})
# endfunction()
