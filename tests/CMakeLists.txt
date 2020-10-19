set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${PROJECT_INC_DIR})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

add_library(Catch2 INTERFACE)
target_include_directories(Catch2 INTERFACE ${CATCH2_FOLDER_PATH})
add_library(FakeIt INTERFACE)
target_include_directories(FakeIt INTERFACE ${FAKEIT_FOLDER_PATH})

add_executable(test-jpod test_main.cc)

target_link_libraries(test-jpod
                      PRIVATE
                      fmt::fmt
                      PUBLIC
                      Catch2
                      FakeIt
                      )

target_include_directories(test-jpod PRIVATE ${PROJECT_INC_DIR})
install(TARGETS test-jpod DESTINATION bin)