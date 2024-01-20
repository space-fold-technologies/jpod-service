
set(LibLZMA_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/xz_utils/5.4.5/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/include")
set(LibLZMA_INCLUDE_DIR_DEBUG "/home/william/.conan/data/xz_utils/5.4.5/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/include")
set(LibLZMA_INCLUDES_DEBUG "/home/william/.conan/data/xz_utils/5.4.5/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/include")
set(LibLZMA_RES_DIRS_DEBUG )
set(LibLZMA_DEFINITIONS_DEBUG "-DLZMA_API_STATIC")
set(LibLZMA_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(LibLZMA_COMPILE_DEFINITIONS_DEBUG "LZMA_API_STATIC")
set(LibLZMA_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(LibLZMA_COMPILE_OPTIONS_C_DEBUG "")
set(LibLZMA_COMPILE_OPTIONS_CXX_DEBUG "")
set(LibLZMA_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(LibLZMA_LIBRARIES_DEBUG "") # Will be filled later
set(LibLZMA_LIBS_DEBUG "") # Same as LibLZMA_LIBRARIES
set(LibLZMA_SYSTEM_LIBS_DEBUG pthread)
set(LibLZMA_FRAMEWORK_DIRS_DEBUG )
set(LibLZMA_FRAMEWORKS_DEBUG )
set(LibLZMA_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(LibLZMA_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(LibLZMA_FRAMEWORKS_FOUND_DEBUG "${LibLZMA_FRAMEWORKS_DEBUG}" "${LibLZMA_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(LibLZMA_INCLUDE_DIRS_DEBUG
                 LibLZMA_INCLUDE_DIR_DEBUG
                 LibLZMA_INCLUDES_DEBUG
                 LibLZMA_DEFINITIONS_DEBUG
                 LibLZMA_LINKER_FLAGS_DEBUG_LIST
                 LibLZMA_COMPILE_DEFINITIONS_DEBUG
                 LibLZMA_COMPILE_OPTIONS_DEBUG_LIST
                 LibLZMA_LIBRARIES_DEBUG
                 LibLZMA_LIBS_DEBUG
                 LibLZMA_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to LibLZMA_LIBS and LibLZMA_LIBRARY_LIST
set(LibLZMA_LIBRARY_LIST_DEBUG lzma)
set(LibLZMA_LIB_DIRS_DEBUG "/home/william/.conan/data/xz_utils/5.4.5/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_LibLZMA_DEPENDENCIES_DEBUG "${LibLZMA_FRAMEWORKS_FOUND_DEBUG} ${LibLZMA_SYSTEM_LIBS_DEBUG} ")

conan_package_library_targets("${LibLZMA_LIBRARY_LIST_DEBUG}"  # libraries
                              "${LibLZMA_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_LibLZMA_DEPENDENCIES_DEBUG}"  # deps
                              LibLZMA_LIBRARIES_DEBUG            # out_libraries
                              LibLZMA_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "LibLZMA")                                      # package_name

set(LibLZMA_LIBS_DEBUG ${LibLZMA_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${LibLZMA_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND LibLZMA_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND LibLZMA_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${LibLZMA_SYSTEM_LIBS_DEBUG})
    list(APPEND LibLZMA_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND LibLZMA_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(LibLZMA_LIBRARIES_TARGETS_DEBUG "${LibLZMA_LIBRARIES_TARGETS_DEBUG};")
set(LibLZMA_LIBRARIES_DEBUG "${LibLZMA_LIBRARIES_DEBUG};")

set(CMAKE_MODULE_PATH "/home/william/.conan/data/xz_utils/5.4.5/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "/home/william/.conan/data/xz_utils/5.4.5/_/_/package/4ff90a381eedd28e01423b1f8744e2217c746aba/" ${CMAKE_PREFIX_PATH})
