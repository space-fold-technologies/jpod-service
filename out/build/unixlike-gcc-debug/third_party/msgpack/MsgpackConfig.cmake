get_filename_component(Subprocess_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

if(NOT TARGET Subprocess)
    include("${Subprocess_CMAKE_DIR}/SubprocessTargets.cmake")
endif()

set(Subprocess_LIBRARIES Subprocess)
