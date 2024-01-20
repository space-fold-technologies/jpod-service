

if(NOT TARGET SQLite::SQLite3)
    add_library(SQLite::SQLite3 INTERFACE IMPORTED)
endif()

if(NOT TARGET SQLite::SQLite)
    add_library(SQLite::SQLite INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/SQLite3Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(SQLite3_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${SQLite3_FIND_COMPONENTS})
        list(FIND SQLite_COMPONENTS_DEBUG "SQLite::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'SQLite'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'SQLite'")
        endif()
    endforeach()
endif()