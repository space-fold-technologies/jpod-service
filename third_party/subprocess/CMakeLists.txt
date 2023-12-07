cmake_minimum_required(VERSION 3.9)
project(subprocess LANGUAGES CXX VERSION 1.0)

if (NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "This project has a top-level one called [${CMAKE_PROJECT_NAME}]")
else()
    message(STATUS "This project is a top-level one")
endif()

find_package(Threads REQUIRED)

add_library(subprocess STATIC)
add_library(subprocess::subprocess ALIAS subprocess)

if(WIN32)
    target_sources(subprocess
        PRIVATE
            src/subprocess/environ.cc
            src/subprocess/pipe_var.cc
            src/subprocess/pipe.cc
            src/subprocess/process_builder.cc 
            src/subprocess/process_builder_windows.cc 
            src/subprocess/shell_utils.cc 
            src/subprocess/utf8_to_utf16.cc
    )
else()
    target_sources(subprocess
        PRIVATE
            src/subprocess/environ.cc
            src/subprocess/pipe_var.cc
            src/subprocess/pipe.cc
            src/subprocess/process_builder.cc 
            src/subprocess/process_builder_posix.cc 
            src/subprocess/shell_utils.cc 
            src/subprocess/utf8_to_utf16.cc
    )
endif()

target_include_directories(subprocess
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# note that it is not CMAKE_INSTALL_PREFIX we are checking here
if(DEFINED CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(
        STATUS
        "CMAKE_INSTALL_PREFIX is not set\n"
        "Default value: ${CMAKE_INSTALL_PREFIX}\n"
        "Will set it to ${CMAKE_SOURCE_DIR}/install"
    )
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_SOURCE_DIR}/install"
        CACHE PATH "Where the library will be installed to" FORCE
    )
else()
    message(
        STATUS
        "CMAKE_INSTALL_PREFIX was already set\n"
        "Current value: ${CMAKE_INSTALL_PREFIX}"
    )
endif()

set (
    public_headers 
    include/subprocess/basic_types.h
    include/subprocess/cow_data.h 
    include/subprocess/environ.h 
    include/subprocess/pipe_var.h 
    include/subprocess/pipe.h 
    include/subprocess/process_builder.h 
    include/subprocess/shell_utils.h 
    include/subprocess/utf8_to_utf16.h 
)

if(MSVC)
    target_compile_options(subprocess PUBLIC -Zc:__cplusplus)
endif()

# not for MSVC
if(CMAKE_COMPILER_IS_GNUCXX)
    # compile options for this target only
    # treat warnings as errors
    target_compile_options(subprocess PRIVATE -Werror)
    target_link_libraries(subprocess PUBLIC stdc++fs)
    if(NOT WIN32)
        target_link_libraries(subprocess PUBLIC dl)
    endif()
endif()
target_link_libraries(subprocess PUBLIC Threads::Threads)
# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#message(STATUS "CMake module path: ${CMAKE_MODULE_PATH}")
include(Installing)
export(EXPORT MsgpackTargets FILE ${CMAKE_CURRENT_BINARY_DIR}/MsgpackTargets.cmake NAMESPACE Msgpack::)
export(PACKAGE Msgpack)