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


set(SQLite_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/include")
set(SQLite_INCLUDE_DIR_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/include")
set(SQLite_INCLUDES_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/include")
set(SQLite_RES_DIRS_DEBUG )
set(SQLite_DEFINITIONS_DEBUG )
set(SQLite_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(SQLite_COMPILE_DEFINITIONS_DEBUG )
set(SQLite_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(SQLite_COMPILE_OPTIONS_C_DEBUG "")
set(SQLite_COMPILE_OPTIONS_CXX_DEBUG "")
set(SQLite_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(SQLite_LIBRARIES_DEBUG "") # Will be filled later
set(SQLite_LIBS_DEBUG "") # Same as SQLite_LIBRARIES
set(SQLite_SYSTEM_LIBS_DEBUG pthread dl m)
set(SQLite_FRAMEWORK_DIRS_DEBUG )
set(SQLite_FRAMEWORKS_DEBUG )
set(SQLite_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(SQLite_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(SQLite_FRAMEWORKS_FOUND_DEBUG "${SQLite_FRAMEWORKS_DEBUG}" "${SQLite_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(SQLite_INCLUDE_DIRS_DEBUG
                 SQLite_INCLUDE_DIR_DEBUG
                 SQLite_INCLUDES_DEBUG
                 SQLite_DEFINITIONS_DEBUG
                 SQLite_LINKER_FLAGS_DEBUG_LIST
                 SQLite_COMPILE_DEFINITIONS_DEBUG
                 SQLite_COMPILE_OPTIONS_DEBUG_LIST
                 SQLite_LIBRARIES_DEBUG
                 SQLite_LIBS_DEBUG
                 SQLite_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to SQLite_LIBS and SQLite_LIBRARY_LIST
set(SQLite_LIBRARY_LIST_DEBUG sqlite3)
set(SQLite_LIB_DIRS_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_SQLite_DEPENDENCIES_DEBUG "${SQLite_FRAMEWORKS_FOUND_DEBUG} ${SQLite_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${SQLite_LIBRARY_LIST_DEBUG}"  # libraries
                              "${SQLite_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_SQLite_DEPENDENCIES_DEBUG}"  # deps
                              SQLite_LIBRARIES_DEBUG            # out_libraries
                              SQLite_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "SQLite")                                      # package_name

set(SQLite_LIBS_DEBUG ${SQLite_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${SQLite_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND SQLite_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND SQLite_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${SQLite_SYSTEM_LIBS_DEBUG})
    list(APPEND SQLite_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND SQLite_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(SQLite_LIBRARIES_TARGETS_DEBUG "${SQLite_LIBRARIES_TARGETS_DEBUG};")
set(SQLite_LIBRARIES_DEBUG "${SQLite_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH})

set(SQLite_COMPONENTS_DEBUG SQLite::SQLite3)

########### COMPONENT SQLite3 VARIABLES #############################################

set(SQLite_SQLite3_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/include")
set(SQLite_SQLite3_INCLUDE_DIR_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/include")
set(SQLite_SQLite3_INCLUDES_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/include")
set(SQLite_SQLite3_LIB_DIRS_DEBUG "/home/william/.conan/data/sqlite3/3.42.0/_/_/package/357c5db193a2be44ea517b01e7557735b098639a/lib")
set(SQLite_SQLite3_RES_DIRS_DEBUG )
set(SQLite_SQLite3_DEFINITIONS_DEBUG )
set(SQLite_SQLite3_COMPILE_DEFINITIONS_DEBUG )
set(SQLite_SQLite3_COMPILE_OPTIONS_C_DEBUG "")
set(SQLite_SQLite3_COMPILE_OPTIONS_CXX_DEBUG "")
set(SQLite_SQLite3_LIBS_DEBUG sqlite3)
set(SQLite_SQLite3_SYSTEM_LIBS_DEBUG pthread dl m)
set(SQLite_SQLite3_FRAMEWORK_DIRS_DEBUG )
set(SQLite_SQLite3_FRAMEWORKS_DEBUG )
set(SQLite_SQLite3_BUILD_MODULES_PATHS_DEBUG )
set(SQLite_SQLite3_DEPENDENCIES_DEBUG )
set(SQLite_SQLite3_LINKER_FLAGS_LIST_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>
)

########## COMPONENT SQLite3 FIND LIBRARIES & FRAMEWORKS / DYNAMIC VARS #############

set(SQLite_SQLite3_FRAMEWORKS_FOUND_DEBUG "")
conan_find_apple_frameworks(SQLite_SQLite3_FRAMEWORKS_FOUND_DEBUG "${SQLite_SQLite3_FRAMEWORKS_DEBUG}" "${SQLite_SQLite3_FRAMEWORK_DIRS_DEBUG}")

set(SQLite_SQLite3_LIB_TARGETS_DEBUG "")
set(SQLite_SQLite3_NOT_USED_DEBUG "")
set(SQLite_SQLite3_LIBS_FRAMEWORKS_DEPS_DEBUG ${SQLite_SQLite3_FRAMEWORKS_FOUND_DEBUG} ${SQLite_SQLite3_SYSTEM_LIBS_DEBUG} ${SQLite_SQLite3_DEPENDENCIES_DEBUG})
conan_package_library_targets("${SQLite_SQLite3_LIBS_DEBUG}"
                              "${SQLite_SQLite3_LIB_DIRS_DEBUG}"
                              "${SQLite_SQLite3_LIBS_FRAMEWORKS_DEPS_DEBUG}"
                              SQLite_SQLite3_NOT_USED_DEBUG
                              SQLite_SQLite3_LIB_TARGETS_DEBUG
                              "DEBUG"
                              "SQLite_SQLite3")

set(SQLite_SQLite3_LINK_LIBS_DEBUG ${SQLite_SQLite3_LIB_TARGETS_DEBUG} ${SQLite_SQLite3_LIBS_FRAMEWORKS_DEPS_DEBUG})