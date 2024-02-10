find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tl-expected REQUIRED CONFIG)
find_package(asio REQUIRED CONFIG)
find_package(OpenSSL REQUIRED CONFIG)
find_package(sole REQUIRED)
find_package(range-v3 REQUIRED CONFIG)
find_package(libzip REQUIRED CONFIG)
find_package(SQLite3 REQUIRED CONFIG)
find_package(zstd REQUIRED CONFIG)
find_package(yaml-cpp REQUIRED CONFIG)
find_package(msgpack REQUIRED CONFIG)

# Generic test that uses conan libs

# if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
#   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASIO_HAS_IO_URING -luring")
# elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")  
#     option(ASIO_HAS_KQUEUE "IO_URING SUPPORT" ON)
# elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS" OR CMAKE_SYSTEM_NAME STREQUAL "SunOS")
#     message(STATUS "Building On Some solaris variant.")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DASIO_HAS_DEV_POLL")
# endif ()

add_executable(daemon
                main.cc
                bootstrap.cc
                core/commands/command_handler.cc
                core/commands/command_handler_registry.cc
                core/connections/connection.cc
                core/connections/connection_acceptor.cc
                core/sql/connection.cc 
                core/sql/data_source.cc 
                core/sql/migrations.cc 
                core/sql/result_set.cc 
                core/sql/statement.cc 
                core/sql/transaction.cc 
                # images
                domain/images/list_handler.cc
                domain/images/build_handler.cc
                domain/images/import_handler.cc
                domain/images/removal_handler.cc
                domain/images/instructions/download_instruction.cc
                domain/images/instructions/copy_instruction.cc
                domain/images/instructions/work_dir_instruction.cc
                domain/images/instructions/run_instruction.cc
                domain/images/instructions/compression_instruction.cc
                domain/images/instructions/mount_instruction.cc
                domain/images/instructions/unmount_instruction.cc
                domain/images/instructions/registration_instruction.cc
                domain/images/instructions/cleanup_instruction.cc
                domain/images/instructions/extraction_instruction.cc
                domain/images/instructions/import_instruction.cc
                domain/images/instructions/build_system_resolver.cc
                domain/images/sql_repository.cc
                domain/images/http/request.cc
                domain/images/http/request_builder.cc
                domain/images/http/connection_base.cc
                domain/images/http/secure_connection.cc
                domain/images/http/insecure_connection.cc
                domain/images/http/asio_client.cc
                #containers
                domain/containers/runtime.cc 
                domain/containers/list_handler.cc
                domain/containers/creation_handler.cc
                domain/containers/start_handler.cc
                domain/containers/logging_handler.cc
                domain/containers/shell_handler.cc
                domain/containers/removal_handler.cc
                domain/containers/sql_repository.cc
                #containers::freebsd
                domain/containers/freebsd/freebsd_container.cc
                domain/containers/freebsd/freebsd_terminal.cc
                # networking
                domain/networking/asio_address_provider.cc
                domain/networking/virtual_network_provider.cc
                )

target_link_libraries(daemon
                       PRIVATE
                       fmt::fmt 
                       spdlog::spdlog_header_only
                       asio::asio
                       range-v3::range-v3
                       tl::expected
                       sole::sole
                       msgpack-cxx
                       libzip::zip
                       zstd::libzstd_static
                       yaml-cpp::yaml-cpp
                       SQLite::SQLite3
                       resource::rc
                       PUBLIC 
                       OpenSSL::Crypto 
                       OpenSSL::SSL
                       )
 
                  
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

    target_link_libraries(daemon PRIVATE jail dl util)   
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS" OR CMAKE_SYSTEM_NAME STREQUAL "Solaris" OR CMAKE_SYSTEM_NAME STREQUAL "Illumos")
    message(STATUS "Building On Solaris OS variant.")
    
endif ()

target_include_directories(daemon
                            PUBLIC
                            ${PROJECT_HDRS_FOLDER}
                            PRIVATE "${CMAKE_BINARY_DIR}"
                            )
install(TARGETS daemon DESTINATION bin)
