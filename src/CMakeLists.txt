find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tl-expected REQUIRED CONFIG)
find_package(asio REQUIRED)
find_package(OpenSSL REQUIRED CONFIG)
find_package(sole REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(range-v3 REQUIRED)
# Generic test that uses conan libs

add_executable(jpod-service
                main.cc
                ops/application.cc
                shell/terminal.cc 
                shell/session.cc
                core/connections/ipc/domain_connection.cc
                core/connections/ipc/domain_connection_acceptor.cc
                core/connections/ws/ws_connection.cc
                core/connections/ws/ws_connection_acceptor.cc
                core/connections/ws/crypto.cc
                core/filesystems/zfs_handler.cc
                # core/networks/interfaces/virtual_network_handler.cc
                # core/networks/pf/query.cc 
                # core/networks/pf/rule_set.cc 
                # core/networks/pf/transaction.cc
                # core/networks/addresses/provider.cc 
                # core/networks/addresses/sql_repository.cc

                # http related stuff
                core/networks/http/request.cc 
                core/networks/http/asio_client.cc
                core/networks/http/connection.cc
                core/networks/http/request_builder.cc
                core/networks/http/file_destination.cc
                # now we add the container related stuff
                images/run_instruction.cc
                images/copy_instruction.cc
                images/from_instruction.cc
                images/work_dir_instruction.cc
                images/image_service.cc
                images/sql_image_repository.cc
                core/databases/connection.cc 
                core/databases/data_source.cc 
                core/databases/migrations.cc 
                core/databases/transaction.cc
                core/databases/result_set.cc 
                core/databases/statement.cc 
                ops/session.cc
                containers/container.cc
                containers/container_runtime.cc
                containers/container_controller.cc
                containers/container_service.cc
                containers/sql_container_repository.cc
                # containers/container_builder.cc
                )

target_link_libraries(jpod-service
                       PRIVATE
                       fmt::fmt 
                       spdlog::spdlog_header_only
                       asio::asio
                       jail
                       dl
                       util
                       Msgpack::Msgpack
                       SQLite::SQLite3
                       tl::expected
                       sole::sole
                       range-v3::range-v3
                       subprocess::subprocess
                       )

target_link_libraries(jpod-service PUBLIC OpenSSL::Crypto)                       

target_include_directories(jpod-service
                            PUBLIC
                            ${PROJECT_HDRS_FOLDER}
                            PRIVATE "${CMAKE_BINARY_DIR}"
                            )
install(TARGETS jpod-service DESTINATION bin)