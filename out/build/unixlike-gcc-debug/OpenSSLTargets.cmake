

if(NOT TARGET OpenSSL::Crypto)
    add_library(OpenSSL::Crypto INTERFACE IMPORTED)
endif()

if(NOT TARGET OpenSSL::SSL)
    add_library(OpenSSL::SSL INTERFACE IMPORTED)
endif()

if(NOT TARGET OpenSSL::OpenSSL)
    add_library(OpenSSL::OpenSSL INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/OpenSSLTarget-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()

if(OpenSSL_FIND_COMPONENTS)
    foreach(_FIND_COMPONENT ${OpenSSL_FIND_COMPONENTS})
        list(FIND OpenSSL_COMPONENTS_DEBUG "OpenSSL::${_FIND_COMPONENT}" _index)
        if(${_index} EQUAL -1)
            conan_message(FATAL_ERROR "Conan: Component '${_FIND_COMPONENT}' NOT found in package 'OpenSSL'")
        else()
            conan_message(STATUS "Conan: Component '${_FIND_COMPONENT}' found in package 'OpenSSL'")
        endif()
    endforeach()
endif()