
set(fakeit_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/fakeit/2.4.0/_/_/package/86d587422df2c31448edbb2a26fabf141995dc78/include")
set(fakeit_INCLUDE_DIR_DEBUG "/home/william/.conan/data/fakeit/2.4.0/_/_/package/86d587422df2c31448edbb2a26fabf141995dc78/include")
set(fakeit_INCLUDES_DEBUG "/home/william/.conan/data/fakeit/2.4.0/_/_/package/86d587422df2c31448edbb2a26fabf141995dc78/include")
set(fakeit_RES_DIRS_DEBUG )
set(fakeit_DEFINITIONS_DEBUG )
set(fakeit_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(fakeit_COMPILE_DEFINITIONS_DEBUG )
set(fakeit_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(fakeit_COMPILE_OPTIONS_C_DEBUG "")
set(fakeit_COMPILE_OPTIONS_CXX_DEBUG "")
set(fakeit_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(fakeit_LIBRARIES_DEBUG "") # Will be filled later
set(fakeit_LIBS_DEBUG "") # Same as fakeit_LIBRARIES
set(fakeit_SYSTEM_LIBS_DEBUG )
set(fakeit_FRAMEWORK_DIRS_DEBUG )
set(fakeit_FRAMEWORKS_DEBUG )
set(fakeit_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(fakeit_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(fakeit_FRAMEWORKS_FOUND_DEBUG "${fakeit_FRAMEWORKS_DEBUG}" "${fakeit_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(fakeit_INCLUDE_DIRS_DEBUG
                 fakeit_INCLUDE_DIR_DEBUG
                 fakeit_INCLUDES_DEBUG
                 fakeit_DEFINITIONS_DEBUG
                 fakeit_LINKER_FLAGS_DEBUG_LIST
                 fakeit_COMPILE_DEFINITIONS_DEBUG
                 fakeit_COMPILE_OPTIONS_DEBUG_LIST
                 fakeit_LIBRARIES_DEBUG
                 fakeit_LIBS_DEBUG
                 fakeit_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to fakeit_LIBS and fakeit_LIBRARY_LIST
set(fakeit_LIBRARY_LIST_DEBUG )
set(fakeit_LIB_DIRS_DEBUG )

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_fakeit_DEPENDENCIES_DEBUG "${fakeit_FRAMEWORKS_FOUND_DEBUG} ${fakeit_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${fakeit_LIBRARY_LIST_DEBUG}"  # libraries
                              "${fakeit_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_fakeit_DEPENDENCIES_DEBUG}"  # deps
                              fakeit_LIBRARIES_DEBUG            # out_libraries
                              fakeit_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "fakeit")                                      # package_name

set(fakeit_LIBS_DEBUG ${fakeit_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${fakeit_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND fakeit_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND fakeit_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${fakeit_SYSTEM_LIBS_DEBUG})
    list(APPEND fakeit_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND fakeit_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(fakeit_LIBRARIES_TARGETS_DEBUG "${fakeit_LIBRARIES_TARGETS_DEBUG};")
set(fakeit_LIBRARIES_DEBUG "${fakeit_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH "/home/william/.conan/data/fakeit/2.4.0/_/_/package/86d587422df2c31448edbb2a26fabf141995dc78/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/william/.conan/data/fakeit/2.4.0/_/_/package/86d587422df2c31448edbb2a26fabf141995dc78/" ${CMAKE_PREFIX_PATH})
