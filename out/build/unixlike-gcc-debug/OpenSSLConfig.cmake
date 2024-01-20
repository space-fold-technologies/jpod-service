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

include(${CMAKE_CURRENT_LIST_DIR}/OpenSSLTargets.cmake)

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

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT Crypto TARGET PROPERTIES ######################################

set_property(TARGET OpenSSL::Crypto PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${OpenSSL_Crypto_LINK_LIBS_DEBUG}
                ${OpenSSL_Crypto_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${OpenSSL_Crypto_LINK_LIBS_RELEASE}
                ${OpenSSL_Crypto_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${OpenSSL_Crypto_LINK_LIBS_RELWITHDEBINFO}
                ${OpenSSL_Crypto_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${OpenSSL_Crypto_LINK_LIBS_MINSIZEREL}
                ${OpenSSL_Crypto_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET OpenSSL::Crypto PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${OpenSSL_Crypto_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${OpenSSL_Crypto_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${OpenSSL_Crypto_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${OpenSSL_Crypto_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET OpenSSL::Crypto PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${OpenSSL_Crypto_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${OpenSSL_Crypto_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${OpenSSL_Crypto_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${OpenSSL_Crypto_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET OpenSSL::Crypto PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_C_DEBUG}
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_C_RELEASE}
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${OpenSSL_Crypto_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(OpenSSL_Crypto_TARGET_PROPERTIES TRUE)

########## COMPONENT SSL TARGET PROPERTIES ######################################

set_property(TARGET OpenSSL::SSL PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${OpenSSL_SSL_LINK_LIBS_DEBUG}
                ${OpenSSL_SSL_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${OpenSSL_SSL_LINK_LIBS_RELEASE}
                ${OpenSSL_SSL_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${OpenSSL_SSL_LINK_LIBS_RELWITHDEBINFO}
                ${OpenSSL_SSL_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${OpenSSL_SSL_LINK_LIBS_MINSIZEREL}
                ${OpenSSL_SSL_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET OpenSSL::SSL PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${OpenSSL_SSL_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${OpenSSL_SSL_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${OpenSSL_SSL_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${OpenSSL_SSL_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET OpenSSL::SSL PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${OpenSSL_SSL_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${OpenSSL_SSL_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${OpenSSL_SSL_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${OpenSSL_SSL_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET OpenSSL::SSL PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${OpenSSL_SSL_COMPILE_OPTIONS_C_DEBUG}
                 ${OpenSSL_SSL_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${OpenSSL_SSL_COMPILE_OPTIONS_C_RELEASE}
                 ${OpenSSL_SSL_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${OpenSSL_SSL_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${OpenSSL_SSL_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${OpenSSL_SSL_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${OpenSSL_SSL_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(OpenSSL_SSL_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT OpenSSL_OpenSSL_TARGET_PROPERTIES)
    set_property(TARGET OpenSSL::OpenSSL APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${OpenSSL_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${OpenSSL_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${OpenSSL_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${OpenSSL_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT Crypto BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${OpenSSL_Crypto_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${OpenSSL_Crypto_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${OpenSSL_Crypto_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${OpenSSL_Crypto_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()

########## COMPONENT SSL BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${OpenSSL_SSL_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${OpenSSL_SSL_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${OpenSSL_SSL_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${OpenSSL_SSL_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()