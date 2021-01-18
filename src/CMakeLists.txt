set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Generated sources
set(SERVICE_PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/protocols.pb.cc")
set(SERVICE_PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/protocols.pb.h")

add_custom_command(
      OUTPUT "${SERVICE_PROTO_SRCS}" "${SERVICE_PROTO_HDRS}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${SERVICE_PROTOS_PATH}"
        "${SERVICE_PROTOS}"
      DEPENDS "${SERVICE_PROTOS}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PROJECT_HDRS_FOLDER}/include)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

add_library(PSTREAMS INTERFACE)
target_include_directories(PSTREAMS INTERFACE ${PSTREAMS_FOLDER_PATH})

add_library(JSON INTERFACE)
target_include_directories(JSON INTERFACE ${NLOHMANN_JSON_FOLDER})

add_executable(${PROJECT_NAME}
                core/shell/terminal.cc
                core/shell/service.cc
                core/shell/session.cc
                main.cc
                ${SERVICE_PROTO_SRCS}
                )

target_link_libraries(${PROJECT_NAME}
                       JSON
                       PSTREAMS
                       spdlog::spdlog
                       jail
                       SQLite::SQLite3
                       dl
                       util
                       ${_PROTOBUF_LIBPROTOBUF}
                       )

target_include_directories(${PROJECT_NAME}
                            PUBLIC
                            ${PROJECT_HDRS_FOLDER}
                            ${SQLite3_INCLUDE_DIRS}
                            ${WEBSOCKETPP_INCLUDE_DIR}
                            )

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)