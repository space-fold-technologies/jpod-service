

if(NOT TARGET range-v3::meta)
    add_library(range-v3::meta INTERFACE IMPORTED)
endif()

if(NOT TARGET range-v3::concepts)
    add_library(range-v3::concepts INTERFACE IMPORTED)
endif()

if(NOT TARGET range-v3::range-v3)
    add_library(range-v3::range-v3 INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/range-v3Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(range-v3_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${range-v3_FIND_COMPONENTS})
        list(FIND range-v3_COMPONENTS_DEBUG "range-v3::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'range-v3'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'range-v3'")
        endif()
    endforeach()
endif()