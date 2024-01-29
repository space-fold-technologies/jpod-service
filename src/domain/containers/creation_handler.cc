#include <domain/containers/creation_handler.h>
#include <domain/containers/runtime.h>
#include <domain/containers/orders.h>

namespace domain::containers
{
    creation_handler::creation_handler(core::connections::connection &connection, std::shared_ptr<runtime> runtime_ptr) : command_handler(connection), runtime_ptr(std::move(runtime_ptr))
    {
    }

    void creation_handler::on_order_received(const std::vector<uint8_t> &payload) 
    {
        auto order = unpack_container_creation_order(payload);
    }
    void creation_handler::on_connection_closed(const std::error_code &error) {}

    creation_handler::~creation_handler() {}

}