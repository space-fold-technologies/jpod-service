
set(asio_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/asio/1.29.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(asio_INCLUDE_DIR_DEBUG "/home/william/.conan/data/asio/1.29.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(asio_INCLUDES_DEBUG "/home/william/.conan/data/asio/1.29.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(asio_RES_DIRS_DEBUG )
set(asio_DEFINITIONS_DEBUG "-DASIO_STANDALONE")
set(asio_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(asio_COMPILE_DEFINITIONS_DEBUG "ASIO_STANDALONE")
set(asio_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(asio_COMPILE_OPTIONS_C_DEBUG "")
set(asio_COMPILE_OPTIONS_CXX_DEBUG "")
set(asio_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(asio_LIBRARIES_DEBUG "") # Will be filled later
set(asio_LIBS_DEBUG "") # Same as asio_LIBRARIES
set(asio_SYSTEM_LIBS_DEBUG pthread)
set(asio_FRAMEWORK_DIRS_DEBUG )
set(asio_FRAMEWORKS_DEBUG )
set(asio_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(asio_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(asio_FRAMEWORKS_FOUND_DEBUG "${asio_FRAMEWORKS_DEBUG}" "${asio_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(asio_INCLUDE_DIRS_DEBUG
                 asio_INCLUDE_DIR_DEBUG
                 asio_INCLUDES_DEBUG
                 asio_DEFINITIONS_DEBUG
                 asio_LINKER_FLAGS_DEBUG_LIST
                 asio_COMPILE_DEFINITIONS_DEBUG
                 asio_COMPILE_OPTIONS_DEBUG_LIST
                 asio_LIBRARIES_DEBUG
                 asio_LIBS_DEBUG
                 asio_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to asio_LIBS and asio_LIBRARY_LIST
set(asio_LIBRARY_LIST_DEBUG )
set(asio_LIB_DIRS_DEBUG )

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_asio_DEPENDENCIES_DEBUG "${asio_FRAMEWORKS_FOUND_DEBUG} ${asio_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${asio_LIBRARY_LIST_DEBUG}"  # libraries
                              "${asio_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_asio_DEPENDENCIES_DEBUG}"  # deps
                              asio_LIBRARIES_DEBUG            # out_libraries
                              asio_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "asio")                                      # package_name

set(asio_LIBS_DEBUG ${asio_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${asio_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND asio_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND asio_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${asio_SYSTEM_LIBS_DEBUG})
    list(APPEND asio_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND asio_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(asio_LIBRARIES_TARGETS_DEBUG "${asio_LIBRARIES_TARGETS_DEBUG};")
set(asio_LIBRARIES_DEBUG "${asio_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH "/home/william/.conan/data/asio/1.29.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/william/.conan/data/asio/1.29.0/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/" ${CMAKE_PREFIX_PATH})
