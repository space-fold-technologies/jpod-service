include_directories(include)

add_library(utilities INTERFACE)
target_link_libraries(utilities INTERFACE archives-resources)
target_include_directories(utilities INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)