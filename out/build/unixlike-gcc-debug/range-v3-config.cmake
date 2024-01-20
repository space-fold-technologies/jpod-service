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

include(${CMAKE_CURRENT_LIST_DIR}/range-v3Targets.cmake)

########## FIND PACKAGE DEPENDENCY ##########################################################
#############################################################################################

include(CMakeFindDependencyMacro)

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT meta TARGET PROPERTIES ######################################

set_property(TARGET range-v3::meta PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${range-v3_meta_LINK_LIBS_DEBUG}
                ${range-v3_meta_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${range-v3_meta_LINK_LIBS_RELEASE}
                ${range-v3_meta_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${range-v3_meta_LINK_LIBS_RELWITHDEBINFO}
                ${range-v3_meta_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${range-v3_meta_LINK_LIBS_MINSIZEREL}
                ${range-v3_meta_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET range-v3::meta PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${range-v3_meta_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${range-v3_meta_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${range-v3_meta_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${range-v3_meta_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET range-v3::meta PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${range-v3_meta_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${range-v3_meta_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${range-v3_meta_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${range-v3_meta_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET range-v3::meta PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${range-v3_meta_COMPILE_OPTIONS_C_DEBUG}
                 ${range-v3_meta_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${range-v3_meta_COMPILE_OPTIONS_C_RELEASE}
                 ${range-v3_meta_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${range-v3_meta_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${range-v3_meta_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${range-v3_meta_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${range-v3_meta_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(range-v3_meta_TARGET_PROPERTIES TRUE)

########## COMPONENT concepts TARGET PROPERTIES ######################################

set_property(TARGET range-v3::concepts PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${range-v3_concepts_LINK_LIBS_DEBUG}
                ${range-v3_concepts_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${range-v3_concepts_LINK_LIBS_RELEASE}
                ${range-v3_concepts_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${range-v3_concepts_LINK_LIBS_RELWITHDEBINFO}
                ${range-v3_concepts_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${range-v3_concepts_LINK_LIBS_MINSIZEREL}
                ${range-v3_concepts_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET range-v3::concepts PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${range-v3_concepts_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${range-v3_concepts_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${range-v3_concepts_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${range-v3_concepts_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET range-v3::concepts PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${range-v3_concepts_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${range-v3_concepts_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${range-v3_concepts_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${range-v3_concepts_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET range-v3::concepts PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${range-v3_concepts_COMPILE_OPTIONS_C_DEBUG}
                 ${range-v3_concepts_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${range-v3_concepts_COMPILE_OPTIONS_C_RELEASE}
                 ${range-v3_concepts_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${range-v3_concepts_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${range-v3_concepts_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${range-v3_concepts_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${range-v3_concepts_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(range-v3_concepts_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT range-v3_range-v3_TARGET_PROPERTIES)
    set_property(TARGET range-v3::range-v3 APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${range-v3_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${range-v3_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${range-v3_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${range-v3_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT meta BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${range-v3_meta_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${range-v3_meta_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${range-v3_meta_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${range-v3_meta_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()

########## COMPONENT concepts BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${range-v3_concepts_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${range-v3_concepts_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${range-v3_concepts_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${range-v3_concepts_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()