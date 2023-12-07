
# Is CMake verbose?
function(is_verbose var)
  if("${CMAKE_MESSAGE_LOG_LEVEL}" STREQUAL "VERBOSE"
     OR "${CMAKE_MESSAGE_LOG_LEVEL}" STREQUAL "DEBUG"
     OR "${CMAKE_MESSAGE_LOG_LEVEL}" STREQUAL "TRACE")
    set(${var}
        ON
        PARENT_SCOPE)
  else()
    set(${var}
        OFF
        PARENT_SCOPE)
  endif()
endfunction()
