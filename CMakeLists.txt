cmake_minimum_required(VERSION 3.16...3.23)

project(jcd)
# Not ideal to use this global variable, but necessary to make sure
# that tooling and projects use the same version
set(CMAKE_CXX_STANDARD 17)

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(utilities)
include(rc)
include(conan)
run_conan()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_TESTS "Build Unit tests" ON)

###################################################################
# ADDED COMPILED RESOURCES                                        #
###################################################################

cmrc_add_resource_library(
    resources
    NAMESPACE resources
    WHENCE resources
    ALIAS resource::rc
    resources/migrations/initial-tables.sql
    resources/migrations/local-registry.sql
    resources/migrations/network-tables.sql
    resources/configurations/settings.yml
    resources/configurations/freebsd-template.yml
    resources/configurations/linux-template.yml
)

include_directories(include)
add_subdirectory(src)

# option(ENABLE_TESTING "Enable the tests" ${PROJECT_IS_TOP_LEVEL})
# if(ENABLE_TESTING)
#     set(PROJECT_HDRS_FOLDER "${CMAKE_CURRENT_LIST_DIR}/include")
#     set(PROJECT_SRC_FOLDER "${CMAKE_CURRENT_LIST_DIR}/src")
#     enable_testing()
#     message(STATUS "Building Tests.")
#     add_subdirectory(tests)
# endif()
