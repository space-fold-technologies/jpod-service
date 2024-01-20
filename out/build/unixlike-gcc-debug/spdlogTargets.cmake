

if(NOT TARGET spdlog::spdlog_header_only)
    add_library(spdlog::spdlog_header_only INTERFACE IMPORTED)
endif()

if(NOT TARGET spdlog::spdlog)
    add_library(spdlog::spdlog INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/spdlogTarget-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(spdlog_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${spdlog_FIND_COMPONENTS})
        list(FIND spdlog_COMPONENTS_DEBUG "spdlog::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'spdlog'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'spdlog'")
        endif()
    endforeach()
endif()