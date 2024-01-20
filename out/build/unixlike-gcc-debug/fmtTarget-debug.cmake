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


set(fmt_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/include")
set(fmt_INCLUDE_DIR_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/include")
set(fmt_INCLUDES_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/include")
set(fmt_RES_DIRS_DEBUG )
set(fmt_DEFINITIONS_DEBUG )
set(fmt_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(fmt_COMPILE_DEFINITIONS_DEBUG )
set(fmt_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(fmt_COMPILE_OPTIONS_C_DEBUG "")
set(fmt_COMPILE_OPTIONS_CXX_DEBUG "")
set(fmt_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(fmt_LIBRARIES_DEBUG "") # Will be filled later
set(fmt_LIBS_DEBUG "") # Same as fmt_LIBRARIES
set(fmt_SYSTEM_LIBS_DEBUG )
set(fmt_FRAMEWORK_DIRS_DEBUG )
set(fmt_FRAMEWORKS_DEBUG )
set(fmt_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(fmt_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(fmt_FRAMEWORKS_FOUND_DEBUG "${fmt_FRAMEWORKS_DEBUG}" "${fmt_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(fmt_INCLUDE_DIRS_DEBUG
                 fmt_INCLUDE_DIR_DEBUG
                 fmt_INCLUDES_DEBUG
                 fmt_DEFINITIONS_DEBUG
                 fmt_LINKER_FLAGS_DEBUG_LIST
                 fmt_COMPILE_DEFINITIONS_DEBUG
                 fmt_COMPILE_OPTIONS_DEBUG_LIST
                 fmt_LIBRARIES_DEBUG
                 fmt_LIBS_DEBUG
                 fmt_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to fmt_LIBS and fmt_LIBRARY_LIST
set(fmt_LIBRARY_LIST_DEBUG fmtd)
set(fmt_LIB_DIRS_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_fmt_DEPENDENCIES_DEBUG "${fmt_FRAMEWORKS_FOUND_DEBUG} ${fmt_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${fmt_LIBRARY_LIST_DEBUG}"  # libraries
                              "${fmt_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_fmt_DEPENDENCIES_DEBUG}"  # deps
                              fmt_LIBRARIES_DEBUG            # out_libraries
                              fmt_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "fmt")                                      # package_name

set(fmt_LIBS_DEBUG ${fmt_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${fmt_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND fmt_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND fmt_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${fmt_SYSTEM_LIBS_DEBUG})
    list(APPEND fmt_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND fmt_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(fmt_LIBRARIES_TARGETS_DEBUG "${fmt_LIBRARIES_TARGETS_DEBUG};")
set(fmt_LIBRARIES_DEBUG "${fmt_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH})

set(fmt_COMPONENTS_DEBUG fmt::fmt)

########### COMPONENT fmt VARIABLES #############################################

set(fmt_fmt_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/include")
set(fmt_fmt_INCLUDE_DIR_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/include")
set(fmt_fmt_INCLUDES_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/include")
set(fmt_fmt_LIB_DIRS_DEBUG "/home/william/.conan/data/fmt/10.1.0/_/_/package/48a53b0d8aab2c762860b0face744f26a231c4b7/lib")
set(fmt_fmt_RES_DIRS_DEBUG )
set(fmt_fmt_DEFINITIONS_DEBUG )
set(fmt_fmt_COMPILE_DEFINITIONS_DEBUG )
set(fmt_fmt_COMPILE_OPTIONS_C_DEBUG "")
set(fmt_fmt_COMPILE_OPTIONS_CXX_DEBUG "")
set(fmt_fmt_LIBS_DEBUG fmtd)
set(fmt_fmt_SYSTEM_LIBS_DEBUG )
set(fmt_fmt_FRAMEWORK_DIRS_DEBUG )
set(fmt_fmt_FRAMEWORKS_DEBUG )
set(fmt_fmt_BUILD_MODULES_PATHS_DEBUG )
set(fmt_fmt_DEPENDENCIES_DEBUG )
set(fmt_fmt_LINKER_FLAGS_LIST_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>
)

########## COMPONENT fmt FIND LIBRARIES & FRAMEWORKS / DYNAMIC VARS #############

set(fmt_fmt_FRAMEWORKS_FOUND_DEBUG "")
conan_find_apple_frameworks(fmt_fmt_FRAMEWORKS_FOUND_DEBUG "${fmt_fmt_FRAMEWORKS_DEBUG}" "${fmt_fmt_FRAMEWORK_DIRS_DEBUG}")

set(fmt_fmt_LIB_TARGETS_DEBUG "")
set(fmt_fmt_NOT_USED_DEBUG "")
set(fmt_fmt_LIBS_FRAMEWORKS_DEPS_DEBUG ${fmt_fmt_FRAMEWORKS_FOUND_DEBUG} ${fmt_fmt_SYSTEM_LIBS_DEBUG} ${fmt_fmt_DEPENDENCIES_DEBUG})
conan_package_library_targets("${fmt_fmt_LIBS_DEBUG}"
                              "${fmt_fmt_LIB_DIRS_DEBUG}"
                              "${fmt_fmt_LIBS_FRAMEWORKS_DEPS_DEBUG}"
                              fmt_fmt_NOT_USED_DEBUG
                              fmt_fmt_LIB_TARGETS_DEBUG
                              "DEBUG"
                              "fmt_fmt")

set(fmt_fmt_LINK_LIBS_DEBUG ${fmt_fmt_LIB_TARGETS_DEBUG} ${fmt_fmt_LIBS_FRAMEWORKS_DEPS_DEBUG})