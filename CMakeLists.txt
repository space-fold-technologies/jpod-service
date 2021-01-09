cmake_minimum_required(VERSION 2.8)

project(jpod-engine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PROJECT_VERSION "1.0.0")
set(CMAKE_PROJECT_VERSION_MAJOR 1)
set(CMAKE_PROJECT_VERSION_MINOR 0)
set(CMAKE_PROJECT_VERSION_PATCH 0)

option(BUILD_TESTS "Build Unit tests" OFF)
include_directories(SYSTEM /usr/local/include)
link_directories(/usr/local/lib)
list(APPEND CMAKE_REQUIRED_INCLUDES /usr/local/include)

find_package(Boost "1.64.0")
find_package(SQLite3 REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

if(Boost_FOUND)
  message(STATUS "Found Boost ${Boost_VERSION}")
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME ON)
else()
  message(WARNING "Failed to find Boost")
endif()

if(SQLite3_FOUND)
  message(STATUS "Found Sqlite ${SQLite3_VERSION}")
else()
  message(WARNING "Could not find Sqlite 3")
endif()

if(Protobuf_FOUND)
 # Find Protobuf installation
   # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
   message(STATUS "Using protobuf ${Protobuf_VERSION}")
   set(Protobuf_USE_STATIC_LIBS ON)
   set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
   set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
endif ()

#########################################################
# ADDING COMPILED PROTOCOL BUFFERS COMPILATION          #
#########################################################

# Proto file
get_filename_component(SERVICE_PROTOS "${CMAKE_CURRENT_LIST_DIR}/protos/protocols.proto" ABSOLUTE)
get_filename_component(SERVICE_PROTOS_PATH "${SERVICE_PROTOS}" PATH)

##################################################################################
# References to source folders in project                                        #
##################################################################################

set(PROJECT_SRC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/src")
set(PROJECT_HDRS_FOLDER "${CMAKE_CURRENT_LIST_DIR}/include")

##################################################################################
# Add Third party libraries in folders                                           #
##################################################################################

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/third_party/spdlog")
set(WEBSOCKETPP_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/third_party/websocketpp")

include_directories(${WEBSOCKETPP_INCLUDE_DIR})

set(PSTREAMS_FOLDER_PATH "${CMAKE_CURRENT_LIST_DIR}/third_party/pstreams")
set(NLOHMANN_JSON_FOLDER "${CMAKE_CURRENT_LIST_DIR}/third_party/json")

##################################################################################
# Adding the actual project source folder                                        #
##################################################################################
add_subdirectory("${PROJECT_SRC_FOLDER}")
include_directories("${PROJECT_HDRS_FOLDER}")

##################################################################################
# Project Unit Tests                                                             #
##################################################################################

if(BUILD_TESTS)
 set(CATCH2_FOLDER_PATH "${CMAKE_CURRENT_LIST_DIR}/third_party/catch2")
 set(FAKEIT_FOLDER_PATH "${CMAKE_CURRENT_LIST_DIR}/third_party/fakeit")
 add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/tests")
endif()
