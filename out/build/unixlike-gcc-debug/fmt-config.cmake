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

include(${CMAKE_CURRENT_LIST_DIR}/fmtTargets.cmake)

########## FIND PACKAGE DEPENDENCY ##########################################################
#############################################################################################

include(CMakeFindDependencyMacro)

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT fmt TARGET PROPERTIES ######################################

set_property(TARGET fmt::fmt PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${fmt_fmt_LINK_LIBS_DEBUG}
                ${fmt_fmt_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${fmt_fmt_LINK_LIBS_RELEASE}
                ${fmt_fmt_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${fmt_fmt_LINK_LIBS_RELWITHDEBINFO}
                ${fmt_fmt_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${fmt_fmt_LINK_LIBS_MINSIZEREL}
                ${fmt_fmt_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET fmt::fmt PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${fmt_fmt_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${fmt_fmt_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${fmt_fmt_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${fmt_fmt_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET fmt::fmt PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${fmt_fmt_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${fmt_fmt_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${fmt_fmt_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${fmt_fmt_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET fmt::fmt PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${fmt_fmt_COMPILE_OPTIONS_C_DEBUG}
                 ${fmt_fmt_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${fmt_fmt_COMPILE_OPTIONS_C_RELEASE}
                 ${fmt_fmt_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${fmt_fmt_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${fmt_fmt_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${fmt_fmt_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${fmt_fmt_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(fmt_fmt_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT fmt_fmt_TARGET_PROPERTIES)
    set_property(TARGET fmt::fmt APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${fmt_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${fmt_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${fmt_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${fmt_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT fmt BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${fmt_fmt_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${fmt_fmt_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${fmt_fmt_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${fmt_fmt_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()