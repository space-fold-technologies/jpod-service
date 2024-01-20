

if(NOT TARGET libzip::zip)
    add_library(libzip::zip INTERFACE IMPORTED)
endif()

if(NOT TARGET libzip::libzip)
    add_library(libzip::libzip INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/libzipTarget-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(libzip_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${libzip_FIND_COMPONENTS})
        list(FIND libzip_COMPONENTS_DEBUG "libzip::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'libzip'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'libzip'")
        endif()
    endforeach()
endif()