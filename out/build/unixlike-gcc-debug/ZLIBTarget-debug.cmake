
set(ZLIB_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/zlib/1.2.13/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/include")
set(ZLIB_INCLUDE_DIR_DEBUG "/home/william/.conan/data/zlib/1.2.13/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/include")
set(ZLIB_INCLUDES_DEBUG "/home/william/.conan/data/zlib/1.2.13/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/include")
set(ZLIB_RES_DIRS_DEBUG )
set(ZLIB_DEFINITIONS_DEBUG )
set(ZLIB_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(ZLIB_COMPILE_DEFINITIONS_DEBUG )
set(ZLIB_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(ZLIB_COMPILE_OPTIONS_C_DEBUG "")
set(ZLIB_COMPILE_OPTIONS_CXX_DEBUG "")
set(ZLIB_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(ZLIB_LIBRARIES_DEBUG "") # Will be filled later
set(ZLIB_LIBS_DEBUG "") # Same as ZLIB_LIBRARIES
set(ZLIB_SYSTEM_LIBS_DEBUG )
set(ZLIB_FRAMEWORK_DIRS_DEBUG )
set(ZLIB_FRAMEWORKS_DEBUG )
set(ZLIB_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(ZLIB_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(ZLIB_FRAMEWORKS_FOUND_DEBUG "${ZLIB_FRAMEWORKS_DEBUG}" "${ZLIB_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(ZLIB_INCLUDE_DIRS_DEBUG
                 ZLIB_INCLUDE_DIR_DEBUG
                 ZLIB_INCLUDES_DEBUG
                 ZLIB_DEFINITIONS_DEBUG
                 ZLIB_LINKER_FLAGS_DEBUG_LIST
                 ZLIB_COMPILE_DEFINITIONS_DEBUG
                 ZLIB_COMPILE_OPTIONS_DEBUG_LIST
                 ZLIB_LIBRARIES_DEBUG
                 ZLIB_LIBS_DEBUG
                 ZLIB_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to ZLIB_LIBS and ZLIB_LIBRARY_LIST
set(ZLIB_LIBRARY_LIST_DEBUG z)
set(ZLIB_LIB_DIRS_DEBUG "/home/william/.conan/data/zlib/1.2.13/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_ZLIB_DEPENDENCIES_DEBUG "${ZLIB_FRAMEWORKS_FOUND_DEBUG} ${ZLIB_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${ZLIB_LIBRARY_LIST_DEBUG}"  # libraries
                              "${ZLIB_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_ZLIB_DEPENDENCIES_DEBUG}"  # deps
                              ZLIB_LIBRARIES_DEBUG            # out_libraries
                              ZLIB_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "ZLIB")                                      # package_name

set(ZLIB_LIBS_DEBUG ${ZLIB_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${ZLIB_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND ZLIB_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND ZLIB_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${ZLIB_SYSTEM_LIBS_DEBUG})
    list(APPEND ZLIB_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND ZLIB_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(ZLIB_LIBRARIES_TARGETS_DEBUG "${ZLIB_LIBRARIES_TARGETS_DEBUG};")
set(ZLIB_LIBRARIES_DEBUG "${ZLIB_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH "/home/william/.conan/data/zlib/1.2.13/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/william/.conan/data/zlib/1.2.13/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/" ${CMAKE_PREFIX_PATH})
