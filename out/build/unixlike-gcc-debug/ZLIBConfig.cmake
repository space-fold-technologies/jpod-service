

function(conan_message MESSAGE_OUTPUT)
    if(NOT CONAN_CMAKE_SILENT_OUTPUT)
        message(${ARGV${0}})
    endif()
endfunction()


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


# Requires CMake > 3.0
if(${CMAKE_VERSION} VERSION_LESS "3.0")
    message(FATAL_ERROR "The 'cmake_find_package_multi' generator only works with CMake > 3.0")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/ZLIBTargets.cmake)


# Assign target properties
set_property(TARGET ZLIB::ZLIB
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${ZLIB_LIBRARIES_TARGETS_DEBUG}
                                    ${ZLIB_LINKER_FLAGS_DEBUG_LIST}>
             $<$<CONFIG:Release>:${ZLIB_LIBRARIES_TARGETS_RELEASE}
                                    ${ZLIB_LINKER_FLAGS_RELEASE_LIST}>
             $<$<CONFIG:RelWithDebInfo>:${ZLIB_LIBRARIES_TARGETS_RELWITHDEBINFO}
                                    ${ZLIB_LINKER_FLAGS_RELWITHDEBINFO_LIST}>
             $<$<CONFIG:MinSizeRel>:${ZLIB_LIBRARIES_TARGETS_MINSIZEREL}
                                    ${ZLIB_LINKER_FLAGS_MINSIZEREL_LIST}>)
set_property(TARGET ZLIB::ZLIB
             PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${ZLIB_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${ZLIB_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${ZLIB_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${ZLIB_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET ZLIB::ZLIB
             PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${ZLIB_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${ZLIB_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${ZLIB_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${ZLIB_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET ZLIB::ZLIB
             PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:${ZLIB_COMPILE_OPTIONS_DEBUG_LIST}>
             $<$<CONFIG:Release>:${ZLIB_COMPILE_OPTIONS_RELEASE_LIST}>
             $<$<CONFIG:RelWithDebInfo>:${ZLIB_COMPILE_OPTIONS_RELWITHDEBINFO_LIST}>
             $<$<CONFIG:MinSizeRel>:${ZLIB_COMPILE_OPTIONS_MINSIZEREL_LIST}>)
    

# Build modules
foreach(_BUILD_MODULE_PATH ${ZLIB_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()
foreach(_BUILD_MODULE_PATH ${ZLIB_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()
foreach(_BUILD_MODULE_PATH ${ZLIB_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()
foreach(_BUILD_MODULE_PATH ${ZLIB_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()
    

