

if(NOT TARGET fmt::fmt)
    add_library(fmt::fmt INTERFACE IMPORTED)
endif()

if(NOT TARGET fmt::fmt)
    add_library(fmt::fmt INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/fmtTarget-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(fmt_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${fmt_FIND_COMPONENTS})
        list(FIND fmt_COMPONENTS_DEBUG "fmt::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'fmt'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'fmt'")
        endif()
    endforeach()
endif()