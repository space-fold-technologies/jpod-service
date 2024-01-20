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

include(${CMAKE_CURRENT_LIST_DIR}/tl-expectedTargets.cmake)

########## FIND PACKAGE DEPENDENCY ##########################################################
#############################################################################################

include(CMakeFindDependencyMacro)

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT expected TARGET PROPERTIES ######################################

set_property(TARGET tl::expected PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${tl_expected_LINK_LIBS_DEBUG}
                ${tl_expected_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${tl_expected_LINK_LIBS_RELEASE}
                ${tl_expected_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${tl_expected_LINK_LIBS_RELWITHDEBINFO}
                ${tl_expected_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${tl_expected_LINK_LIBS_MINSIZEREL}
                ${tl_expected_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET tl::expected PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${tl_expected_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${tl_expected_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${tl_expected_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${tl_expected_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET tl::expected PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${tl_expected_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${tl_expected_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${tl_expected_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${tl_expected_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET tl::expected PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${tl_expected_COMPILE_OPTIONS_C_DEBUG}
                 ${tl_expected_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${tl_expected_COMPILE_OPTIONS_C_RELEASE}
                 ${tl_expected_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${tl_expected_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${tl_expected_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${tl_expected_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${tl_expected_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(tl_expected_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT tl_tl_TARGET_PROPERTIES)
    set_property(TARGET tl::tl APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${tl_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${tl_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${tl_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${tl_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT expected BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${tl_expected_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${tl_expected_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${tl_expected_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${tl_expected_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()