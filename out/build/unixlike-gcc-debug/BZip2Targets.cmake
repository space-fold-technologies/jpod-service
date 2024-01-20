
if(NOT TARGET BZip2::BZip2)
    add_library(BZip2::BZip2 INTERFACE IMPORTED)
endif()

# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/BZip2Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()
