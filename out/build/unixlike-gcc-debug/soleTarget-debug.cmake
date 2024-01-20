
set(sole_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/sole/1.0.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(sole_INCLUDE_DIR_DEBUG "/home/william/.conan/data/sole/1.0.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(sole_INCLUDES_DEBUG "/home/william/.conan/data/sole/1.0.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(sole_RES_DIRS_DEBUG )
set(sole_DEFINITIONS_DEBUG )
set(sole_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(sole_COMPILE_DEFINITIONS_DEBUG )
set(sole_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(sole_COMPILE_OPTIONS_C_DEBUG "")
set(sole_COMPILE_OPTIONS_CXX_DEBUG "")
set(sole_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(sole_LIBRARIES_DEBUG "") # Will be filled later
set(sole_LIBS_DEBUG "") # Same as sole_LIBRARIES
set(sole_SYSTEM_LIBS_DEBUG rt)
set(sole_FRAMEWORK_DIRS_DEBUG )
set(sole_FRAMEWORKS_DEBUG )
set(sole_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(sole_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(sole_FRAMEWORKS_FOUND_DEBUG "${sole_FRAMEWORKS_DEBUG}" "${sole_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(sole_INCLUDE_DIRS_DEBUG
                 sole_INCLUDE_DIR_DEBUG
                 sole_INCLUDES_DEBUG
                 sole_DEFINITIONS_DEBUG
                 sole_LINKER_FLAGS_DEBUG_LIST
                 sole_COMPILE_DEFINITIONS_DEBUG
                 sole_COMPILE_OPTIONS_DEBUG_LIST
                 sole_LIBRARIES_DEBUG
                 sole_LIBS_DEBUG
                 sole_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to sole_LIBS and sole_LIBRARY_LIST
set(sole_LIBRARY_LIST_DEBUG )
set(sole_LIB_DIRS_DEBUG )

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_sole_DEPENDENCIES_DEBUG "${sole_FRAMEWORKS_FOUND_DEBUG} ${sole_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${sole_LIBRARY_LIST_DEBUG}"  # libraries
                              "${sole_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_sole_DEPENDENCIES_DEBUG}"  # deps
                              sole_LIBRARIES_DEBUG            # out_libraries
                              sole_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "sole")                                      # package_name

set(sole_LIBS_DEBUG ${sole_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${sole_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND sole_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND sole_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${sole_SYSTEM_LIBS_DEBUG})
    list(APPEND sole_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND sole_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(sole_LIBRARIES_TARGETS_DEBUG "${sole_LIBRARIES_TARGETS_DEBUG};")
set(sole_LIBRARIES_DEBUG "${sole_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH "/home/william/.conan/data/sole/1.0.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/william/.conan/data/sole/1.0.2/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/" ${CMAKE_PREFIX_PATH})
