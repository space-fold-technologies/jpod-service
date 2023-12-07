cmake_minimum_required(VERSION 3.15...3.23)

include_directories(${PROJECT_HDRS_FOLDER})

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Catch2 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include(Catch)

add_library(catch_main OBJECT catch_main.cc)
target_link_libraries(catch_main PUBLIC spdlog::spdlog Catch2::Catch2)

# image tests 
add_subdirectory(images)