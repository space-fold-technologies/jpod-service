

if(NOT TARGET tl::expected)
    add_library(tl::expected INTERFACE IMPORTED)
endif()

if(NOT TARGET tl::tl)
    add_library(tl::tl INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/tl-expectedTarget-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(tl-expected_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${tl-expected_FIND_COMPONENTS})
        list(FIND tl_COMPONENTS_DEBUG "tl::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'tl'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'tl'")
        endif()
    endforeach()
endif()