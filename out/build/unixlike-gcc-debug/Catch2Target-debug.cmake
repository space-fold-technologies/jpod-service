
set(Catch2_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/catch2/2.13.9/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(Catch2_INCLUDE_DIR_DEBUG "/home/william/.conan/data/catch2/2.13.9/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(Catch2_INCLUDES_DEBUG "/home/william/.conan/data/catch2/2.13.9/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/include")
set(Catch2_RES_DIRS_DEBUG )
set(Catch2_DEFINITIONS_DEBUG )
set(Catch2_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(Catch2_COMPILE_DEFINITIONS_DEBUG )
set(Catch2_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(Catch2_COMPILE_OPTIONS_C_DEBUG "")
set(Catch2_COMPILE_OPTIONS_CXX_DEBUG "")
set(Catch2_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(Catch2_LIBRARIES_DEBUG "") # Will be filled later
set(Catch2_LIBS_DEBUG "") # Same as Catch2_LIBRARIES
set(Catch2_SYSTEM_LIBS_DEBUG )
set(Catch2_FRAMEWORK_DIRS_DEBUG )
set(Catch2_FRAMEWORKS_DEBUG )
set(Catch2_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(Catch2_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(Catch2_FRAMEWORKS_FOUND_DEBUG "${Catch2_FRAMEWORKS_DEBUG}" "${Catch2_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(Catch2_INCLUDE_DIRS_DEBUG
                 Catch2_INCLUDE_DIR_DEBUG
                 Catch2_INCLUDES_DEBUG
                 Catch2_DEFINITIONS_DEBUG
                 Catch2_LINKER_FLAGS_DEBUG_LIST
                 Catch2_COMPILE_DEFINITIONS_DEBUG
                 Catch2_COMPILE_OPTIONS_DEBUG_LIST
                 Catch2_LIBRARIES_DEBUG
                 Catch2_LIBS_DEBUG
                 Catch2_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to Catch2_LIBS and Catch2_LIBRARY_LIST
set(Catch2_LIBRARY_LIST_DEBUG )
set(Catch2_LIB_DIRS_DEBUG "/home/william/.conan/data/catch2/2.13.9/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_Catch2_DEPENDENCIES_DEBUG "${Catch2_FRAMEWORKS_FOUND_DEBUG} ${Catch2_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${Catch2_LIBRARY_LIST_DEBUG}"  # libraries
                              "${Catch2_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_Catch2_DEPENDENCIES_DEBUG}"  # deps
                              Catch2_LIBRARIES_DEBUG            # out_libraries
                              Catch2_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "Catch2")                                      # package_name

set(Catch2_LIBS_DEBUG ${Catch2_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${Catch2_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND Catch2_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND Catch2_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${Catch2_SYSTEM_LIBS_DEBUG})
    list(APPEND Catch2_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND Catch2_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(Catch2_LIBRARIES_TARGETS_DEBUG "${Catch2_LIBRARIES_TARGETS_DEBUG};")
set(Catch2_LIBRARIES_DEBUG "${Catch2_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH "/home/william/.conan/data/catch2/2.13.9/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/lib/cmake/Catch2" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/william/.conan/data/catch2/2.13.9/_/_/package/5ab84d6acfe1f23c4fae0ab88f26e3a396351ac9/lib/cmake/Catch2" ${CMAKE_PREFIX_PATH})
