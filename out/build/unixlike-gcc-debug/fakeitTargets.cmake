
if(NOT TARGET fakeit::fakeit)
    add_library(fakeit::fakeit INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/fakeitTarget-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()
