########## MACROS ###########################################################################
#############################################################################################


macro(conan_find_apple_frameworks FRAMEWORKS_FOUND FRAMEWORKS FRAMEWORKS_DIRS)
    if(APPLE)
        foreach(_FRAMEWORK ${FRAMEWORKS})
            # https://cmake.org/pipermail/cmake-developers/2017-August/030199.html
            find_library(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND NAMES ${_FRAMEWORK} PATHS ${FRAMEWORKS_DIRS} CMAKE_FIND_ROOT_PATH_BOTH)
            if(CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND)
                list(APPEND ${FRAMEWORKS_FOUND} ${CONAN_FRAMEWORK_${_FRAMEWORK}_FOUND})
            else()
                message(FATAL_ERROR "Framework library ${_FRAMEWORK} not found in paths: ${FRAMEWORKS_DIRS}")
            endif()
        endforeach()
    endif()
endmacro()


function(conan_package_library_targets libraries package_libdir deps out_libraries out_libraries_target build_type package_name)
    unset(_CONAN_ACTUAL_TARGETS CACHE)
    unset(_CONAN_FOUND_SYSTEM_LIBS CACHE)
    foreach(_LIBRARY_NAME ${libraries})
        find_library(CONAN_FOUND_LIBRARY NAMES ${_LIBRARY_NAME} PATHS ${package_libdir}
                     NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
        if(CONAN_FOUND_LIBRARY)
            conan_message(STATUS "Library ${_LIBRARY_NAME} found ${CONAN_FOUND_LIBRARY}")
            list(APPEND _out_libraries ${CONAN_FOUND_LIBRARY})
            if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
                # Create a micro-target for each lib/a found
                string(REGEX REPLACE "[^A-Za-z0-9.+_-]" "_" _LIBRARY_NAME ${_LIBRARY_NAME})
                set(_LIB_NAME CONAN_LIB::${package_name}_${_LIBRARY_NAME}${build_type})
                if(NOT TARGET ${_LIB_NAME})
                    # Create a micro-target for each lib/a found
                    add_library(${_LIB_NAME} UNKNOWN IMPORTED)
                    set_target_properties(${_LIB_NAME} PROPERTIES IMPORTED_LOCATION ${CONAN_FOUND_LIBRARY})
                    set(_CONAN_ACTUAL_TARGETS ${_CONAN_ACTUAL_TARGETS} ${_LIB_NAME})
                else()
                    conan_message(STATUS "Skipping already existing target: ${_LIB_NAME}")
                endif()
                list(APPEND _out_libraries_target ${_LIB_NAME})
            endif()
            conan_message(STATUS "Found: ${CONAN_FOUND_LIBRARY}")
        else()
            conan_message(STATUS "Library ${_LIBRARY_NAME} not found in package, might be system one")
            list(APPEND _out_libraries_target ${_LIBRARY_NAME})
            list(APPEND _out_libraries ${_LIBRARY_NAME})
            set(_CONAN_FOUND_SYSTEM_LIBS "${_CONAN_FOUND_SYSTEM_LIBS};${_LIBRARY_NAME}")
        endif()
        unset(CONAN_FOUND_LIBRARY CACHE)
    endforeach()

    if(NOT ${CMAKE_VERSION} VERSION_LESS "3.0")
        # Add all dependencies to all targets
        string(REPLACE " " ";" deps_list "${deps}")
        foreach(_CONAN_ACTUAL_TARGET ${_CONAN_ACTUAL_TARGETS})
            set_property(TARGET ${_CONAN_ACTUAL_TARGET} PROPERTY INTERFACE_LINK_LIBRARIES "${_CONAN_FOUND_SYSTEM_LIBS};${deps_list}")
        endforeach()
    endif()

    set(${out_libraries} ${_out_libraries} PARENT_SCOPE)
    set(${out_libraries_target} ${_out_libraries_target} PARENT_SCOPE)
endfunction()


########### VARIABLES #######################################################################
#############################################################################################


set(range-v3_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_INCLUDE_DIR_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_INCLUDES_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_RES_DIRS_DEBUG )
set(range-v3_DEFINITIONS_DEBUG )
set(range-v3_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(range-v3_COMPILE_DEFINITIONS_DEBUG )
set(range-v3_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(range-v3_COMPILE_OPTIONS_C_DEBUG "")
set(range-v3_COMPILE_OPTIONS_CXX_DEBUG "")
set(range-v3_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(range-v3_LIBRARIES_DEBUG "") # Will be filled later
set(range-v3_LIBS_DEBUG "") # Same as range-v3_LIBRARIES
set(range-v3_SYSTEM_LIBS_DEBUG )
set(range-v3_FRAMEWORK_DIRS_DEBUG )
set(range-v3_FRAMEWORKS_DEBUG )
set(range-v3_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(range-v3_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(range-v3_FRAMEWORKS_FOUND_DEBUG "${range-v3_FRAMEWORKS_DEBUG}" "${range-v3_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(range-v3_INCLUDE_DIRS_DEBUG
                 range-v3_INCLUDE_DIR_DEBUG
                 range-v3_INCLUDES_DEBUG
                 range-v3_DEFINITIONS_DEBUG
                 range-v3_LINKER_FLAGS_DEBUG_LIST
                 range-v3_COMPILE_DEFINITIONS_DEBUG
                 range-v3_COMPILE_OPTIONS_DEBUG_LIST
                 range-v3_LIBRARIES_DEBUG
                 range-v3_LIBS_DEBUG
                 range-v3_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to range-v3_LIBS and range-v3_LIBRARY_LIST
set(range-v3_LIBRARY_LIST_DEBUG )
set(range-v3_LIB_DIRS_DEBUG )

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_range-v3_DEPENDENCIES_DEBUG "${range-v3_FRAMEWORKS_FOUND_DEBUG} ${range-v3_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${range-v3_LIBRARY_LIST_DEBUG}"  # libraries
                              "${range-v3_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_range-v3_DEPENDENCIES_DEBUG}"  # deps
                              range-v3_LIBRARIES_DEBUG            # out_libraries
                              range-v3_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "range-v3")                                      # package_name

set(range-v3_LIBS_DEBUG ${range-v3_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${range-v3_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND range-v3_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND range-v3_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${range-v3_SYSTEM_LIBS_DEBUG})
    list(APPEND range-v3_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND range-v3_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(range-v3_LIBRARIES_TARGETS_DEBUG "${range-v3_LIBRARIES_TARGETS_DEBUG};")
set(range-v3_LIBRARIES_DEBUG "${range-v3_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH})

set(range-v3_COMPONENTS_DEBUG range-v3::concepts range-v3::meta)

########### COMPONENT meta VARIABLES #############################################

set(range-v3_meta_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_meta_INCLUDE_DIR_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_meta_INCLUDES_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_meta_LIB_DIRS_DEBUG )
set(range-v3_meta_RES_DIRS_DEBUG )
set(range-v3_meta_DEFINITIONS_DEBUG )
set(range-v3_meta_COMPILE_DEFINITIONS_DEBUG )
set(range-v3_meta_COMPILE_OPTIONS_C_DEBUG "")
set(range-v3_meta_COMPILE_OPTIONS_CXX_DEBUG "")
set(range-v3_meta_LIBS_DEBUG )
set(range-v3_meta_SYSTEM_LIBS_DEBUG )
set(range-v3_meta_FRAMEWORK_DIRS_DEBUG )
set(range-v3_meta_FRAMEWORKS_DEBUG )
set(range-v3_meta_BUILD_MODULES_PATHS_DEBUG )
set(range-v3_meta_DEPENDENCIES_DEBUG )
set(range-v3_meta_LINKER_FLAGS_LIST_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>
)

########## COMPONENT meta FIND LIBRARIES & FRAMEWORKS / DYNAMIC VARS #############

set(range-v3_meta_FRAMEWORKS_FOUND_DEBUG "")
conan_find_apple_frameworks(range-v3_meta_FRAMEWORKS_FOUND_DEBUG "${range-v3_meta_FRAMEWORKS_DEBUG}" "${range-v3_meta_FRAMEWORK_DIRS_DEBUG}")

set(range-v3_meta_LIB_TARGETS_DEBUG "")
set(range-v3_meta_NOT_USED_DEBUG "")
set(range-v3_meta_LIBS_FRAMEWORKS_DEPS_DEBUG ${range-v3_meta_FRAMEWORKS_FOUND_DEBUG} ${range-v3_meta_SYSTEM_LIBS_DEBUG} ${range-v3_meta_DEPENDENCIES_DEBUG})
conan_package_library_targets("${range-v3_meta_LIBS_DEBUG}"
                              "${range-v3_meta_LIB_DIRS_DEBUG}"
                              "${range-v3_meta_LIBS_FRAMEWORKS_DEPS_DEBUG}"
                              range-v3_meta_NOT_USED_DEBUG
                              range-v3_meta_LIB_TARGETS_DEBUG
                              "DEBUG"
                              "range-v3_meta")

set(range-v3_meta_LINK_LIBS_DEBUG ${range-v3_meta_LIB_TARGETS_DEBUG} ${range-v3_meta_LIBS_FRAMEWORKS_DEPS_DEBUG})

########### COMPONENT concepts VARIABLES #############################################

set(range-v3_concepts_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_concepts_INCLUDE_DIR_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_concepts_INCLUDES_DEBUG "/home/william/.conan/data/range-v3/0.12.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(range-v3_concepts_LIB_DIRS_DEBUG )
set(range-v3_concepts_RES_DIRS_DEBUG )
set(range-v3_concepts_DEFINITIONS_DEBUG )
set(range-v3_concepts_COMPILE_DEFINITIONS_DEBUG )
set(range-v3_concepts_COMPILE_OPTIONS_C_DEBUG "")
set(range-v3_concepts_COMPILE_OPTIONS_CXX_DEBUG "")
set(range-v3_concepts_LIBS_DEBUG )
set(range-v3_concepts_SYSTEM_LIBS_DEBUG )
set(range-v3_concepts_FRAMEWORK_DIRS_DEBUG )
set(range-v3_concepts_FRAMEWORKS_DEBUG )
set(range-v3_concepts_BUILD_MODULES_PATHS_DEBUG )
set(range-v3_concepts_DEPENDENCIES_DEBUG range-v3::meta)
set(range-v3_concepts_LINKER_FLAGS_LIST_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>
)

########## COMPONENT concepts FIND LIBRARIES & FRAMEWORKS / DYNAMIC VARS #############

set(range-v3_concepts_FRAMEWORKS_FOUND_DEBUG "")
conan_find_apple_frameworks(range-v3_concepts_FRAMEWORKS_FOUND_DEBUG "${range-v3_concepts_FRAMEWORKS_DEBUG}" "${range-v3_concepts_FRAMEWORK_DIRS_DEBUG}")

set(range-v3_concepts_LIB_TARGETS_DEBUG "")
set(range-v3_concepts_NOT_USED_DEBUG "")
set(range-v3_concepts_LIBS_FRAMEWORKS_DEPS_DEBUG ${range-v3_concepts_FRAMEWORKS_FOUND_DEBUG} ${range-v3_concepts_SYSTEM_LIBS_DEBUG} ${range-v3_concepts_DEPENDENCIES_DEBUG})
conan_package_library_targets("${range-v3_concepts_LIBS_DEBUG}"
                              "${range-v3_concepts_LIB_DIRS_DEBUG}"
                              "${range-v3_concepts_LIBS_FRAMEWORKS_DEPS_DEBUG}"
                              range-v3_concepts_NOT_USED_DEBUG
                              range-v3_concepts_LIB_TARGETS_DEBUG
                              "DEBUG"
                              "range-v3_concepts")

set(range-v3_concepts_LINK_LIBS_DEBUG ${range-v3_concepts_LIB_TARGETS_DEBUG} ${range-v3_concepts_LIBS_FRAMEWORKS_DEPS_DEBUG})