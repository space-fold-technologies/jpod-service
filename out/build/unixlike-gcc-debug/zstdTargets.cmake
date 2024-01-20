

if(NOT TARGET zstd::libzstd_static)
    add_library(zstd::libzstd_static INTERFACE IMPORTED)
endif()

if(NOT TARGET zstd::zstd)
    add_library(zstd::zstd INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/zstdTarget-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(zstd_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${zstd_FIND_COMPONENTS})
        list(FIND zstd_COMPONENTS_DEBUG "zstd::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'zstd'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'zstd'")
        endif()
    endforeach()
endif()