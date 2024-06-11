find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(asio REQUIRED CONFIG)
find_package(OpenSSL REQUIRED CONFIG)
find_package(sole REQUIRED)
find_package(range-v3 REQUIRED CONFIG)
find_package(LibArchive REQUIRED CONFIG)
find_package(SQLite3 REQUIRED CONFIG)
find_package(zstd REQUIRED CONFIG)
find_package(yaml-cpp REQUIRED CONFIG)
find_package(msgpack REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)
find_package(llhttp REQUIRED CONFIG)
find_package(tl-expected REQUIRED CONFIG)

add_executable(jcd
                main.cc
                bootstrap.cc
                # core::oci
                core/oci/oci_client.cc
                core/oci/layer_download_task.cc
                # core::http
                core/http/secure_session.cc
                core/http/insecure_session.cc
                core/http/rest_client.cc
                core/http/file_transfer_client.cc
                # core::commands
                core/commands/command_handler.cc
                core/commands/command_handler_registry.cc
                # core::connections
                core/connections/connection.cc
                core/connections/connection_acceptor.cc
                # core::sql
                core/sql/connection.cc 
                core/sql/data_source.cc 
                core/sql/migrations.cc 
                core/sql/result_set.cc 
                core/sql/statement.cc 
                core/sql/transaction.cc 
                # core::archives
                core/archives/helper.cc
                # domain::images
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
                domain/images/instructions/build_system_resolver.cc
                domain/images/sql_repository.cc
                #domain::containers
                domain/containers/runtime.cc 
                domain/containers/list_handler.cc
                domain/containers/creation_handler.cc
                domain/containers/start_handler.cc
                domain/containers/stop_handler.cc
                domain/containers/logging_handler.cc
                domain/containers/shell_handler.cc
                domain/containers/removal_handler.cc
                domain/containers/sql_repository.cc
                domain/containers/in_memory_monitor.cc
                # domain::networking
                domain/networking/address_provider.cc
                domain/networking/network_service.cc
                domain/networking/create_handler.cc 
                domain/networking/list_handler.cc 
                domain/networking/remove_handler.cc
                domain/networking/sql_repository.cc
                )

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        message(STATUS "source files for FreeBSD OS variant.")
        target_sources(jcd
                       PRIVATE
                       #containers::freebsd
                        domain/containers/freebsd/freebsd_container.cc
                        domain/containers/freebsd/freebsd_terminal.cc
                       # networking::freebsd
                        domain/networking/freebsd/freebsd_network_handler.cc
                      )   
# elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#         message(STATUS "source files for Linux Based OS variant.")
#         target_sources(jcd
#                         PRIVATE
#                         #containers::linux
#                         domain/containers/linux/linux_container.cc
#                         domain/containers/linux/linux_terminal.cc
#                         # networking::linux
#                         # domain/containers/linux/linux_vnet_provider.cc
#                         )                
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS" OR CMAKE_SYSTEM_NAME STREQUAL "Solaris" OR CMAKE_SYSTEM_NAME STREQUAL "Illumos")
        message(STATUS "source files for Illumos OS variant.")        
endif ()

target_link_libraries(jcd
                       PRIVATE
                       fmt::fmt 
                       spdlog::spdlog_header_only
                       asio::asio
                       range-v3::range-v3
                       sole::sole
                       msgpack-cxx
                       nlohmann_json::nlohmann_json
                       LibArchive::LibArchive
                       zstd::libzstd_static
                       yaml-cpp::yaml-cpp
                       SQLite::SQLite3
                       resource::rc
                       llhttp::llhttp
                       PUBLIC 
                       OpenSSL::Crypto 
                       OpenSSL::SSL
                       CLI11::CLI11
                       tl::expected
                       )
 
                  
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

    target_link_libraries(jcd PRIVATE jail dl util)
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS" OR CMAKE_SYSTEM_NAME STREQUAL "Solaris" OR CMAKE_SYSTEM_NAME STREQUAL "Illumos")
    message(STATUS "Building On Solaris OS variant.")
endif ()

target_include_directories(jcd
                            PUBLIC
                            ${PROJECT_HDRS_FOLDER}
                            PRIVATE "${CMAKE_BINARY_DIR}"
                            )
install(TARGETS jcd DESTINATION bin)
