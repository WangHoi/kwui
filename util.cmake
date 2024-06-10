# MIT License
# 
# Copyright (c) 2019 Cristian Adam 
# Copyright (c) 2021 William O'Neill
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

function(bundle_static_library tgt_name bundled_tgt_name)
    list(APPEND static_libs ${tgt_name})

    function(_recursively_collect_dependencies input_target)
        set(_input_link_libraries LINK_LIBRARIES)
        get_target_property(_input_type ${input_target} TYPE)
        if (${_input_type} STREQUAL "INTERFACE_LIBRARY")
            set(_input_link_libraries INTERFACE_LINK_LIBRARIES)
        endif ()
        get_target_property(public_dependencies ${input_target} ${_input_link_libraries})
        foreach (dep IN LISTS public_dependencies)
            # Added to support out-of-directory target link library settings. Not sure
            # if this should need extra processing of the source directory, but it is
            # working for us when we make sure bundle_static_library is called last
            string(REGEX REPLACE "::@.*" "" dependency ${dep})
            if (TARGET ${dependency})
                get_target_property(alias ${dependency} ALIASED_TARGET)
                if (TARGET ${alias})
                    set(dependency ${alias})
                endif ()
                get_target_property(_type ${dependency} TYPE)
                if (${_type} STREQUAL "STATIC_LIBRARY")
                    list(APPEND static_libs ${dependency})
                endif ()
                if (${_type} STREQUAL "OBJECT_LIBRARY")
                    list(APPEND object_libs ${dependency})
                endif ()

                get_property(library_already_added
                        GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency})
                if (NOT library_already_added)
                    set_property(GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency} ON)
                    _recursively_collect_dependencies(${dependency})
                endif ()
            endif ()
        endforeach ()
        set(static_libs ${static_libs} PARENT_SCOPE)
        set(object_libs ${object_libs} PARENT_SCOPE)
    endfunction()

    _recursively_collect_dependencies(${tgt_name})

    list(REMOVE_DUPLICATES static_libs)
    list(REMOVE_DUPLICATES object_libs)

    set(bundled_tgt_full_name
            ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${bundled_tgt_name}${CMAKE_STATIC_LIBRARY_SUFFIX})

    if (CMAKE_CXX_COMPILER_ID MATCHES "(Clang|GNU)")
        file(WRITE ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
                "CREATE ${bundled_tgt_full_name}\n")

        foreach (tgt IN LISTS static_libs)
            file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
                    "ADDLIB $<TARGET_FILE:${tgt}>\n")
        endforeach ()

        foreach (tgt IN LISTS object_libs)
            file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in
                    "ADDMOD $<TARGET_OBJECTS:${tgt}>\n")
        endforeach ()

        file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "SAVE\n")
        file(APPEND ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in "END\n")

        file(GENERATE
                OUTPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
                INPUT ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar.in)

        set(ar_tool ${CMAKE_AR})
        set(ranlib_tool ${CMAKE_RANLIB})
        if (CMAKE_INTERPROCEDURAL_OPTIMIZATION)
            set(ar_tool ${CMAKE_CXX_COMPILER_AR})
            set(ranlib_tool ${CMAKE_CXX_COMPILER_RANLIB})
        endif ()

        # TODO: confirm ranlib is required (adding this b/c of objlib additions)
        add_custom_command(
                COMMAND ${ar_tool} -M < ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar && ${ranlib_tool} ${bundled_tgt_full_name}
                OUTPUT ${bundled_tgt_full_name}
                COMMENT "Bundling ${bundled_tgt_name}"
                VERBATIM)
    elseif (MSVC)
        # IMPORTANT: objlib support NOT added for MSVC
        # presumably something needs done with object_libs list
        find_program(lib_tool lib)

        foreach (tgt IN LISTS static_libs)
            list(APPEND static_libs_full_names $<TARGET_FILE:${tgt}>)
        endforeach ()

        #[[ add_custom_target(${bundled_tgt_name}_target ALL DEPENDS ${bundled_tgt_full_name}})
        add_custom_target(${bundled_tgt_name}_target
          COMMAND ${lib_tool} /NOLOGO /OUT:${bundled_tgt_full_name} ${static_libs_full_names}
          DEPENDS ${static_libs})
        ]]
        add_custom_command(TARGET ${tgt_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Bundling ${bundled_tgt_name} library..."
                COMMAND ${lib_tool} /NOLOGO /OUT:${bundled_tgt_full_name} ${static_libs_full_names}
                VERBATIM)
    else ()
        message(FATAL_ERROR "Unknown bundle scenario!")
    endif ()

    add_library(${bundled_tgt_name} INTERFACE)
    target_link_libraries(${bundled_tgt_name} INTERFACE ${tgt_name})
    add_dependencies(${bundled_tgt_name} ${bundled_tgt_name}_target)

    install(FILES ${bundled_tgt_full_name} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

endfunction()

##################################################
## Macros
##################################################

##################################################
## MACRO: "unrimp_replace_cmake_cxx_flags()" for replacing content inside "CMAKE_C_FLAGS" as well as "CMAKE_CXX_FLAGS"
#
# Usage example: "unrimp_replace_cmake_cxx_flags(/EHsc /EHs-c-)"
##################################################
macro(unrimp_replace_cmake_cxx_flags from to)
    # Basing on https://stackoverflow.com/a/14172871
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
    )
    foreach (CompilerFlag ${CompilerFlags})
        string(REPLACE "${from}" "${to}" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach ()
endmacro()

macro(make_resource_header FILENAME VARNAME)
    set(infile "${CMAKE_SOURCE_DIR}/src/resources/${FILENAME}")
    set(outfile "${CMAKE_CURRENT_BINARY_DIR}/${VARNAME}.txt")

    add_custom_command(OUTPUT "${outfile}"
            COMMAND ${CMAKE_COMMAND} -D "INPUT_FILE=${infile}" -D "OUTPUT_FILE=${outfile}"
            -P "${CMAKE_MODULE_PATH}/bin2h.script.cmake"
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            DEPENDS "${infile}" "${CMAKE_MODULE_PATH}/bin2h.script.cmake"
            VERBATIM
    )
    set(ALC_OBJS ${ALC_OBJS} "${outfile}")
endmacro()

function(kwui_add_executable tgt_name)
    if (ANDROID)
        apk_add_executable(${tgt_name})
        target_link_libraries(${tgt_name} kwui)
    else ()
        add_executable(${tgt_name})
        target_link_libraries(${tgt_name} kwui)
        add_custom_command(TARGET ${tgt_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:${tgt_name}> $<TARGET_FILE_DIR:${tgt_name}>
                COMMAND_EXPAND_LISTS
        )
    endif ()
endfunction()
