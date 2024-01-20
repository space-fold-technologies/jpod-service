########## MACROS ###########################################################################
#############################################################################################

function(conan_message MESSAGE_OUTPUT)
    if(NOT CONAN_CMAKE_SILENT_OUTPUT)
        message(${ARGV${0}})
    endif()
endfunction()


# Requires CMake > 3.0
if(${CMAKE_VERSION} VERSION_LESS "3.0")
    message(FATAL_ERROR "The 'cmake_find_package_multi' generator only works with CMake > 3.0")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/libzipTargets.cmake)

########## FIND PACKAGE DEPENDENCY ##########################################################
#############################################################################################

include(CMakeFindDependencyMacro)

if(NOT ZLIB_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS "3.9.0")
        find_package(ZLIB REQUIRED NO_MODULE)
    else()
        find_dependency(ZLIB REQUIRED NO_MODULE)
    endif()
else()
    message(STATUS "Dependency ZLIB already found")
endif()

if(NOT BZip2_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS "3.9.0")
        find_package(BZip2 REQUIRED NO_MODULE)
    else()
        find_dependency(BZip2 REQUIRED NO_MODULE)
    endif()
else()
    message(STATUS "Dependency BZip2 already found")
endif()

if(NOT LibLZMA_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS "3.9.0")
        find_package(LibLZMA REQUIRED NO_MODULE)
    else()
        find_dependency(LibLZMA REQUIRED NO_MODULE)
    endif()
else()
    message(STATUS "Dependency LibLZMA already found")
endif()

if(NOT zstd_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS "3.9.0")
        find_package(zstd REQUIRED NO_MODULE)
    else()
        find_dependency(zstd REQUIRED NO_MODULE)
    endif()
else()
    message(STATUS "Dependency zstd already found")
endif()

if(NOT OpenSSL_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS "3.9.0")
        find_package(OpenSSL REQUIRED NO_MODULE)
    else()
        find_dependency(OpenSSL REQUIRED NO_MODULE)
    endif()
else()
    message(STATUS "Dependency OpenSSL already found")
endif()

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT zip TARGET PROPERTIES ######################################

set_property(TARGET libzip::zip PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${libzip_zip_LINK_LIBS_DEBUG}
                ${libzip_zip_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${libzip_zip_LINK_LIBS_RELEASE}
                ${libzip_zip_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${libzip_zip_LINK_LIBS_RELWITHDEBINFO}
                ${libzip_zip_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${libzip_zip_LINK_LIBS_MINSIZEREL}
                ${libzip_zip_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET libzip::zip PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${libzip_zip_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${libzip_zip_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${libzip_zip_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${libzip_zip_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET libzip::zip PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${libzip_zip_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${libzip_zip_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${libzip_zip_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${libzip_zip_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET libzip::zip PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${libzip_zip_COMPILE_OPTIONS_C_DEBUG}
                 ${libzip_zip_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${libzip_zip_COMPILE_OPTIONS_C_RELEASE}
                 ${libzip_zip_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${libzip_zip_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${libzip_zip_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${libzip_zip_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${libzip_zip_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(libzip_zip_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT libzip_libzip_TARGET_PROPERTIES)
    set_property(TARGET libzip::libzip APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${libzip_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${libzip_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${libzip_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${libzip_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT zip BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${libzip_zip_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${libzip_zip_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${libzip_zip_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${libzip_zip_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()