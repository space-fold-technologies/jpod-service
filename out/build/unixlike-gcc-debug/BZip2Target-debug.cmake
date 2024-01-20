
set(BZip2_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/bzip2/1.0.8/_/_/package/c94da3b50d318eead1089667864c1e9ca276409a/include")
set(BZip2_INCLUDE_DIR_DEBUG "/home/william/.conan/data/bzip2/1.0.8/_/_/package/c94da3b50d318eead1089667864c1e9ca276409a/include")
set(BZip2_INCLUDES_DEBUG "/home/william/.conan/data/bzip2/1.0.8/_/_/package/c94da3b50d318eead1089667864c1e9ca276409a/include")
set(BZip2_RES_DIRS_DEBUG )
set(BZip2_DEFINITIONS_DEBUG )
set(BZip2_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(BZip2_COMPILE_DEFINITIONS_DEBUG )
set(BZip2_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(BZip2_COMPILE_OPTIONS_C_DEBUG "")
set(BZip2_COMPILE_OPTIONS_CXX_DEBUG "")
set(BZip2_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(BZip2_LIBRARIES_DEBUG "") # Will be filled later
set(BZip2_LIBS_DEBUG "") # Same as BZip2_LIBRARIES
set(BZip2_SYSTEM_LIBS_DEBUG )
set(BZip2_FRAMEWORK_DIRS_DEBUG )
set(BZip2_FRAMEWORKS_DEBUG )
set(BZip2_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(BZip2_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(BZip2_FRAMEWORKS_FOUND_DEBUG "${BZip2_FRAMEWORKS_DEBUG}" "${BZip2_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(BZip2_INCLUDE_DIRS_DEBUG
                 BZip2_INCLUDE_DIR_DEBUG
                 BZip2_INCLUDES_DEBUG
                 BZip2_DEFINITIONS_DEBUG
                 BZip2_LINKER_FLAGS_DEBUG_LIST
                 BZip2_COMPILE_DEFINITIONS_DEBUG
                 BZip2_COMPILE_OPTIONS_DEBUG_LIST
                 BZip2_LIBRARIES_DEBUG
                 BZip2_LIBS_DEBUG
                 BZip2_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to BZip2_LIBS and BZip2_LIBRARY_LIST
set(BZip2_LIBRARY_LIST_DEBUG bz2)
set(BZip2_LIB_DIRS_DEBUG "/home/william/.conan/data/bzip2/1.0.8/_/_/package/c94da3b50d318eead1089667864c1e9ca276409a/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_BZip2_DEPENDENCIES_DEBUG "${BZip2_FRAMEWORKS_FOUND_DEBUG} ${BZip2_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${BZip2_LIBRARY_LIST_DEBUG}"  # libraries
                              "${BZip2_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_BZip2_DEPENDENCIES_DEBUG}"  # deps
                              BZip2_LIBRARIES_DEBUG            # out_libraries
                              BZip2_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "BZip2")                                      # package_name

set(BZip2_LIBS_DEBUG ${BZip2_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${BZip2_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND BZip2_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND BZip2_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${BZip2_SYSTEM_LIBS_DEBUG})
    list(APPEND BZip2_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND BZip2_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(BZip2_LIBRARIES_TARGETS_DEBUG "${BZip2_LIBRARIES_TARGETS_DEBUG};")
set(BZip2_LIBRARIES_DEBUG "${BZip2_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH "/home/william/.conan/data/bzip2/1.0.8/_/_/package/c94da3b50d318eead1089667864c1e9ca276409a/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/william/.conan/data/bzip2/1.0.8/_/_/package/c94da3b50d318eead1089667864c1e9ca276409a/" ${CMAKE_PREFIX_PATH})
