
if(NOT TARGET Catch2::Catch2)
    add_library(Catch2::Catch2 INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/Catch2Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()
