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

include(${CMAKE_CURRENT_LIST_DIR}/SQLite3Targets.cmake)

########## FIND PACKAGE DEPENDENCY ##########################################################
#############################################################################################

include(CMakeFindDependencyMacro)

########## TARGETS PROPERTIES ###############################################################
#############################################################################################

########## COMPONENT SQLite3 TARGET PROPERTIES ######################################

set_property(TARGET SQLite::SQLite3 PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${SQLite_SQLite3_LINK_LIBS_DEBUG}
                ${SQLite_SQLite3_LINKER_FLAGS_LIST_DEBUG}>
             $<$<CONFIG:Release>:${SQLite_SQLite3_LINK_LIBS_RELEASE}
                ${SQLite_SQLite3_LINKER_FLAGS_LIST_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${SQLite_SQLite3_LINK_LIBS_RELWITHDEBINFO}
                ${SQLite_SQLite3_LINKER_FLAGS_LIST_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${SQLite_SQLite3_LINK_LIBS_MINSIZEREL}
                ${SQLite_SQLite3_LINKER_FLAGS_LIST_MINSIZEREL}>)
set_property(TARGET SQLite::SQLite3 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
             $<$<CONFIG:Debug>:${SQLite_SQLite3_INCLUDE_DIRS_DEBUG}>
             $<$<CONFIG:Release>:${SQLite_SQLite3_INCLUDE_DIRS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${SQLite_SQLite3_INCLUDE_DIRS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${SQLite_SQLite3_INCLUDE_DIRS_MINSIZEREL}>)
set_property(TARGET SQLite::SQLite3 PROPERTY INTERFACE_COMPILE_DEFINITIONS
             $<$<CONFIG:Debug>:${SQLite_SQLite3_COMPILE_DEFINITIONS_DEBUG}>
             $<$<CONFIG:Release>:${SQLite_SQLite3_COMPILE_DEFINITIONS_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:${SQLite_SQLite3_COMPILE_DEFINITIONS_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:${SQLite_SQLite3_COMPILE_DEFINITIONS_MINSIZEREL}>)
set_property(TARGET SQLite::SQLite3 PROPERTY INTERFACE_COMPILE_OPTIONS
             $<$<CONFIG:Debug>:
                 ${SQLite_SQLite3_COMPILE_OPTIONS_C_DEBUG}
                 ${SQLite_SQLite3_COMPILE_OPTIONS_CXX_DEBUG}>
             $<$<CONFIG:Release>:
                 ${SQLite_SQLite3_COMPILE_OPTIONS_C_RELEASE}
                 ${SQLite_SQLite3_COMPILE_OPTIONS_CXX_RELEASE}>
             $<$<CONFIG:RelWithDebInfo>:
                 ${SQLite_SQLite3_COMPILE_OPTIONS_C_RELWITHDEBINFO}
                 ${SQLite_SQLite3_COMPILE_OPTIONS_CXX_RELWITHDEBINFO}>
             $<$<CONFIG:MinSizeRel>:
                 ${SQLite_SQLite3_COMPILE_OPTIONS_C_MINSIZEREL}
                 ${SQLite_SQLite3_COMPILE_OPTIONS_CXX_MINSIZEREL}>)
set(SQLite_SQLite3_TARGET_PROPERTIES TRUE)

########## GLOBAL TARGET PROPERTIES #########################################################

if(NOT SQLite_SQLite_TARGET_PROPERTIES)
    set_property(TARGET SQLite::SQLite APPEND PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${SQLite_COMPONENTS_DEBUG}>
                 $<$<CONFIG:Release>:${SQLite_COMPONENTS_RELEASE}>
                 $<$<CONFIG:RelWithDebInfo>:${SQLite_COMPONENTS_RELWITHDEBINFO}>
                 $<$<CONFIG:MinSizeRel>:${SQLite_COMPONENTS_MINSIZEREL}>)
endif()

########## BUILD MODULES ####################################################################
#############################################################################################

########## COMPONENT SQLite3 BUILD MODULES ##########################################

foreach(_BUILD_MODULE_PATH ${SQLite_SQLite3_BUILD_MODULES_PATHS_DEBUG})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${SQLite_SQLite3_BUILD_MODULES_PATHS_RELEASE})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${SQLite_SQLite3_BUILD_MODULES_PATHS_RELWITHDEBINFO})
    include(${_BUILD_MODULE_PATH})
endforeach()

foreach(_BUILD_MODULE_PATH ${SQLite_SQLite3_BUILD_MODULES_PATHS_MINSIZEREL})
    include(${_BUILD_MODULE_PATH})
endforeach()