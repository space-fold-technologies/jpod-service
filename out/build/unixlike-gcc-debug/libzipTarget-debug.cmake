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


set(libzip_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/include")
set(libzip_INCLUDE_DIR_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/include")
set(libzip_INCLUDES_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/include")
set(libzip_RES_DIRS_DEBUG )
set(libzip_DEFINITIONS_DEBUG )
set(libzip_LINKER_FLAGS_DEBUG_LIST
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>"
        "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>"
)
set(libzip_COMPILE_DEFINITIONS_DEBUG )
set(libzip_COMPILE_OPTIONS_DEBUG_LIST "" "")
set(libzip_COMPILE_OPTIONS_C_DEBUG "")
set(libzip_COMPILE_OPTIONS_CXX_DEBUG "")
set(libzip_LIBRARIES_TARGETS_DEBUG "") # Will be filled later, if CMake 3
set(libzip_LIBRARIES_DEBUG "") # Will be filled later
set(libzip_LIBS_DEBUG "") # Same as libzip_LIBRARIES
set(libzip_SYSTEM_LIBS_DEBUG )
set(libzip_FRAMEWORK_DIRS_DEBUG )
set(libzip_FRAMEWORKS_DEBUG )
set(libzip_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
set(libzip_BUILD_MODULES_PATHS_DEBUG )

conan_find_apple_frameworks(libzip_FRAMEWORKS_FOUND_DEBUG "${libzip_FRAMEWORKS_DEBUG}" "${libzip_FRAMEWORK_DIRS_DEBUG}")

mark_as_advanced(libzip_INCLUDE_DIRS_DEBUG
                 libzip_INCLUDE_DIR_DEBUG
                 libzip_INCLUDES_DEBUG
                 libzip_DEFINITIONS_DEBUG
                 libzip_LINKER_FLAGS_DEBUG_LIST
                 libzip_COMPILE_DEFINITIONS_DEBUG
                 libzip_COMPILE_OPTIONS_DEBUG_LIST
                 libzip_LIBRARIES_DEBUG
                 libzip_LIBS_DEBUG
                 libzip_LIBRARIES_TARGETS_DEBUG)

# Find the real .lib/.a and add them to libzip_LIBS and libzip_LIBRARY_LIST
set(libzip_LIBRARY_LIST_DEBUG zip)
set(libzip_LIB_DIRS_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/lib")

# Gather all the libraries that should be linked to the targets (do not touch existing variables):
set(_libzip_DEPENDENCIES_DEBUG "${libzip_FRAMEWORKS_FOUND_DEBUG} ${libzip_SYSTEM_LIBS_DEBUG} ZLIB::ZLIB;BZip2::BZip2;LibLZMA::LibLZMA;zstd::zstd;OpenSSL::Crypto")

conan_package_library_targets("${libzip_LIBRARY_LIST_DEBUG}"  # libraries
                              "${libzip_LIB_DIRS_DEBUG}"      # package_libdir
                              "${_libzip_DEPENDENCIES_DEBUG}"  # deps
                              libzip_LIBRARIES_DEBUG            # out_libraries
                              libzip_LIBRARIES_TARGETS_DEBUG    # out_libraries_targets
                              "_DEBUG"                          # build_type
                              "libzip")                                      # package_name

set(libzip_LIBS_DEBUG ${libzip_LIBRARIES_DEBUG})

foreach(_FRAMEWORK ${libzip_FRAMEWORKS_FOUND_DEBUG})
    list(APPEND libzip_LIBRARIES_TARGETS_DEBUG ${_FRAMEWORK})
    list(APPEND libzip_LIBRARIES_DEBUG ${_FRAMEWORK})
endforeach()

foreach(_SYSTEM_LIB ${libzip_SYSTEM_LIBS_DEBUG})
    list(APPEND libzip_LIBRARIES_TARGETS_DEBUG ${_SYSTEM_LIB})
    list(APPEND libzip_LIBRARIES_DEBUG ${_SYSTEM_LIB})
endforeach()

# We need to add our requirements too
set(libzip_LIBRARIES_TARGETS_DEBUG "${libzip_LIBRARIES_TARGETS_DEBUG};ZLIB::ZLIB;BZip2::BZip2;LibLZMA::LibLZMA;zstd::zstd;OpenSSL::Crypto")
set(libzip_LIBRARIES_DEBUG "${libzip_LIBRARIES_DEBUG};ZLIB::ZLIB;BZip2::BZip2;LibLZMA::LibLZMA;zstd::zstd;OpenSSL::Crypto")

set(CMAKE_MODULE_PATH  ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH})

set(libzip_COMPONENTS_DEBUG libzip::zip)

########### COMPONENT zip VARIABLES #############################################

set(libzip_zip_INCLUDE_DIRS_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/include")
set(libzip_zip_INCLUDE_DIR_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/include")
set(libzip_zip_INCLUDES_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/include")
set(libzip_zip_LIB_DIRS_DEBUG "/home/william/.conan/data/libzip/1.10.1/_/_/package/c8c0ee893f9f362cf4d1f09245d8594d9ac6626f/lib")
set(libzip_zip_RES_DIRS_DEBUG )
set(libzip_zip_DEFINITIONS_DEBUG )
set(libzip_zip_COMPILE_DEFINITIONS_DEBUG )
set(libzip_zip_COMPILE_OPTIONS_C_DEBUG "")
set(libzip_zip_COMPILE_OPTIONS_CXX_DEBUG "")
set(libzip_zip_LIBS_DEBUG zip)
set(libzip_zip_SYSTEM_LIBS_DEBUG )
set(libzip_zip_FRAMEWORK_DIRS_DEBUG )
set(libzip_zip_FRAMEWORKS_DEBUG )
set(libzip_zip_BUILD_MODULES_PATHS_DEBUG )
set(libzip_zip_DEPENDENCIES_DEBUG ZLIB::ZLIB BZip2::BZip2 LibLZMA::LibLZMA zstd::zstd OpenSSL::Crypto)
set(libzip_zip_LINKER_FLAGS_LIST_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:>
)

########## COMPONENT zip FIND LIBRARIES & FRAMEWORKS / DYNAMIC VARS #############

set(libzip_zip_FRAMEWORKS_FOUND_DEBUG "")
conan_find_apple_frameworks(libzip_zip_FRAMEWORKS_FOUND_DEBUG "${libzip_zip_FRAMEWORKS_DEBUG}" "${libzip_zip_FRAMEWORK_DIRS_DEBUG}")

set(libzip_zip_LIB_TARGETS_DEBUG "")
set(libzip_zip_NOT_USED_DEBUG "")
set(libzip_zip_LIBS_FRAMEWORKS_DEPS_DEBUG ${libzip_zip_FRAMEWORKS_FOUND_DEBUG} ${libzip_zip_SYSTEM_LIBS_DEBUG} ${libzip_zip_DEPENDENCIES_DEBUG})
conan_package_library_targets("${libzip_zip_LIBS_DEBUG}"
                              "${libzip_zip_LIB_DIRS_DEBUG}"
                              "${libzip_zip_LIBS_FRAMEWORKS_DEPS_DEBUG}"
                              libzip_zip_NOT_USED_DEBUG
                              libzip_zip_LIB_TARGETS_DEBUG
                              "DEBUG"
                              "libzip_zip")

set(libzip_zip_LINK_LIBS_DEBUG ${libzip_zip_LIB_TARGETS_DEBUG} ${libzip_zip_LIBS_FRAMEWORKS_DEPS_DEBUG})