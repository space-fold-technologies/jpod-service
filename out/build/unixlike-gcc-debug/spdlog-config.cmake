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

include(${CMAKE_CURRENT_LIST_DIR}/spdlogTargets.cmake)

########## FIND PACKAGE DEPENDENCY ##########################################################
#############################################################################################

include(CMakeFindDependencyMacro)

if(NOT fmt_FOUND)
    if(${CMAKE_VERSION} VERSION_LESS "3.9.0")
        find_package(fmt REQUIRED NO_MODULE)
    else()
        find_dependency(fmt REQUIRED NO_MODULE)
    endif()
else()
    message(STATUS "Dependency fmt already found")
endif()

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT spdlog_header_only TARGET PROPERTIES ######################################

set_property(TARGET spdlog::spdlog_header_only PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${spdlog_spdlog_header_only_LINK_LIBS_DEBUG}
                ${spdlog_spdlog_header_only_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${spdlog_spdlog_header_only_LINK_LIBS_RELEASE}
                ${spdlog_spdlog_header_only_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${spdlog_spdlog_header_only_LINK_LIBS_RELWITHDEBINFO}
                ${spdlog_spdlog_header_only_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${spdlog_spdlog_header_only_LINK_LIBS_MINSIZEREL}
                ${spdlog_spdlog_header_only_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET spdlog::spdlog_header_only PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${spdlog_spdlog_header_only_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${spdlog_spdlog_header_only_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${spdlog_spdlog_header_only_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${spdlog_spdlog_header_only_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET spdlog::spdlog_header_only PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${spdlog_spdlog_header_only_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${spdlog_spdlog_header_only_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${spdlog_spdlog_header_only_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${spdlog_spdlog_header_only_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET spdlog::spdlog_header_only PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_C_DEBUG}
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_C_RELEASE}
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${spdlog_spdlog_header_only_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(spdlog_spdlog_header_only_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT spdlog_spdlog_TARGET_PROPERTIES)
    set_property(TARGET spdlog::spdlog APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${spdlog_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${spdlog_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${spdlog_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${spdlog_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT spdlog_header_only BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${spdlog_spdlog_header_only_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${spdlog_spdlog_header_only_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${spdlog_spdlog_header_only_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${spdlog_spdlog_header_only_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()