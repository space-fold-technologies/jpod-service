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

include(${CMAKE_CURRENT_LIST_DIR}/zstdTargets.cmake)

########## FIND PACKAGE DEPENDENCY ##########################################################
#############################################################################################

include(CMakeFindDependencyMacro)

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT libzstd_static TARGET PROPERTIES ######################################

set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${zstd_libzstd_static_LINK_LIBS_DEBUG}
                ${zstd_libzstd_static_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${zstd_libzstd_static_LINK_LIBS_RELEASE}
                ${zstd_libzstd_static_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${zstd_libzstd_static_LINK_LIBS_RELWITHDEBINFO}
                ${zstd_libzstd_static_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${zstd_libzstd_static_LINK_LIBS_MINSIZEREL}
                ${zstd_libzstd_static_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${zstd_libzstd_static_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${zstd_libzstd_static_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${zstd_libzstd_static_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${zstd_libzstd_static_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${zstd_libzstd_static_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${zstd_libzstd_static_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${zstd_libzstd_static_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${zstd_libzstd_static_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${zstd_libzstd_static_COMPILE_OPTIONS_C_DEBUG}
                 ${zstd_libzstd_static_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${zstd_libzstd_static_COMPILE_OPTIONS_C_RELEASE}
                 ${zstd_libzstd_static_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${zstd_libzstd_static_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${zstd_libzstd_static_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${zstd_libzstd_static_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${zstd_libzstd_static_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(zstd_libzstd_static_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT zstd_zstd_TARGET_PROPERTIES)
    set_property(TARGET zstd::zstd APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${zstd_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${zstd_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${zstd_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${zstd_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT libzstd_static BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${zstd_libzstd_static_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${zstd_libzstd_static_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${zstd_libzstd_static_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${zstd_libzstd_static_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()