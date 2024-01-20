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


set(zstd_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/include")
set(zstd_INCLUDE_DIR_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/include")
set(zstd_INCLUDES_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/include")
set(zstd_RES_DIRS_DEBUG )
set(zstd_DEFINITIONS_DEBUG )
set(zstd_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(zstd_COMPILE_DEFINITIONS_DEBUG )
set(zstd_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(zstd_COMPILE_OPTIONS_C_DEBUG "")
set(zstd_COMPILE_OPTIONS_CXX_DEBUG "")
set(zstd_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(zstd_LIBRARIES_DEBUG "") # Will be filled later
set(zstd_LIBS_DEBUG "") # Same as zstd_LIBRARIES
set(zstd_SYSTEM_LIBS_DEBUG pthread)
set(zstd_FRAMEWORK_DIRS_DEBUG )
set(zstd_FRAMEWORKS_DEBUG )
set(zstd_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(zstd_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(zstd_FRAMEWORKS_FOUND_DEBUG "${zstd_FRAMEWORKS_DEBUG}" "${zstd_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(zstd_INCLUDE_DIRS_DEBUG
                 zstd_INCLUDE_DIR_DEBUG
                 zstd_INCLUDES_DEBUG
                 zstd_DEFINITIONS_DEBUG
                 zstd_LINKER_FLAGS_DEBUG_LIST
                 zstd_COMPILE_DEFINITIONS_DEBUG
                 zstd_COMPILE_OPTIONS_DEBUG_LIST
                 zstd_LIBRARIES_DEBUG
                 zstd_LIBS_DEBUG
                 zstd_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to zstd_LIBS and zstd_LIBRARY_LIST
set(zstd_LIBRARY_LIST_DEBUG zstd)
set(zstd_LIB_DIRS_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_zstd_DEPENDENCIES_DEBUG "${zstd_FRAMEWORKS_FOUND_DEBUG} ${zstd_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${zstd_LIBRARY_LIST_DEBUG}"  # libraries
                              "${zstd_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_zstd_DEPENDENCIES_DEBUG}"  # deps
                              zstd_LIBRARIES_DEBUG            # out_libraries
                              zstd_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "zstd")                                      # package_name

set(zstd_LIBS_DEBUG ${zstd_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${zstd_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND zstd_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND zstd_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${zstd_SYSTEM_LIBS_DEBUG})
    list(APPEND zstd_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND zstd_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(zstd_LIBRARIES_TARGETS_DEBUG "${zstd_LIBRARIES_TARGETS_DEBUG};")
set(zstd_LIBRARIES_DEBUG "${zstd_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH})

set(zstd_COMPONENTS_DEBUG zstd::libzstd_static)

########### COMPONENT libzstd_static VARIABLES #############################################

set(zstd_libzstd_static_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/include")
set(zstd_libzstd_static_INCLUDE_DIR_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/include")
set(zstd_libzstd_static_INCLUDES_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/include")
set(zstd_libzstd_static_LIB_DIRS_DEBUG "/home/william/.conan/data/zstd/1.5.5/_/_/package/b99e1b5e11cad06467d6262fb1caf52a2d97b018/lib")
set(zstd_libzstd_static_RES_DIRS_DEBUG )
set(zstd_libzstd_static_DEFINITIONS_DEBUG )
set(zstd_libzstd_static_COMPILE_DEFINITIONS_DEBUG )
set(zstd_libzstd_static_COMPILE_OPTIONS_C_DEBUG "")
set(zstd_libzstd_static_COMPILE_OPTIONS_CXX_DEBUG "")
set(zstd_libzstd_static_LIBS_DEBUG zstd)
set(zstd_libzstd_static_SYSTEM_LIBS_DEBUG pthread)
set(zstd_libzstd_static_FRAMEWORK_DIRS_DEBUG )
set(zstd_libzstd_static_FRAMEWORKS_DEBUG )
set(zstd_libzstd_static_BUILD_MODULES_PATHS_DEBUG )
set(zstd_libzstd_static_DEPENDENCIES_DEBUG )
set(zstd_libzstd_static_LINKER_FLAGS_LIST_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>
)

########## COMPONENT libzstd_static FIND LIBRARIES & FRAMEWORKS / DYNAMIC VARS #############

set(zstd_libzstd_static_FRAMEWORKS_FOUND_DEBUG "")
conan_find_apple_frameworks(zstd_libzstd_static_FRAMEWORKS_FOUND_DEBUG "${zstd_libzstd_static_FRAMEWORKS_DEBUG}" "${zstd_libzstd_static_FRAMEWORK_DIRS_DEBUG}")

set(zstd_libzstd_static_LIB_TARGETS_DEBUG "")
set(zstd_libzstd_static_NOT_USED_DEBUG "")
set(zstd_libzstd_static_LIBS_FRAMEWORKS_DEPS_DEBUG ${zstd_libzstd_static_FRAMEWORKS_FOUND_DEBUG} ${zstd_libzstd_static_SYSTEM_LIBS_DEBUG} ${zstd_libzstd_static_DEPENDENCIES_DEBUG})
conan_package_library_targets("${zstd_libzstd_static_LIBS_DEBUG}"
                              "${zstd_libzstd_static_LIB_DIRS_DEBUG}"
                              "${zstd_libzstd_static_LIBS_FRAMEWORKS_DEPS_DEBUG}"
                              zstd_libzstd_static_NOT_USED_DEBUG
                              zstd_libzstd_static_LIB_TARGETS_DEBUG
                              "DEBUG"
                              "zstd_libzstd_static")

set(zstd_libzstd_static_LINK_LIBS_DEBUG ${zstd_libzstd_static_LIB_TARGETS_DEBUG} ${zstd_libzstd_static_LIBS_FRAMEWORKS_DEPS_DEBUG})