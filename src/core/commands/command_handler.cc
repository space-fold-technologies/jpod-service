#include <core/commands/command_handler.h>
#include <core/commands/internal_contracts.h>
#include <core/connections/connection.h>
#include <core/connections/details.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <random>

using namespace core::connections;
namespace core::commands
{
    command_handler::command_handler(core::connections::connection &connection) : connection(connection), logger(spdlog::get("jpod"))
    {
    }

    void command_handler::send_error(const std::error_code &err)
    {
        auto error = error_payload{err.message()};
        auto payload = pack_error_payload(error);
        connection.write(static_cast<uint8_t>(operation_target::client), static_cast<uint8_t>(response_operation::failure), payload);
    }
    void command_handler::send_error(const std::string &err)
    {
        auto error = error_payload{err};
        auto payload = pack_error_payload(error);
        connection.write(static_cast<uint8_t>(operation_target::client), static_cast<uint8_t>(response_operation::failure), payload);
    }
    void command_handler::send_frame(const std::vector<uint8_t> &payload)
    {
        connection.write(static_cast<uint8_t>(operation_target::client), static_cast<uint8_t>(response_operation::frame), payload);
    }
    void command_handler::send_success(const std::string &message)
    {
        auto payload = success_payload{message};
        auto success = pack_success_payload(payload);
        connection.write(static_cast<uint8_t>(operation_target::client), static_cast<uint8_t>(response_operation::success), success);
    }
    void command_handler::send_progress(const std::vector<uint8_t> &data)
    {
        connection.write(static_cast<uint8_t>(operation_target::client), static_cast<uint8_t>(response_operation::progress), data);
    }
    void command_handler::send_close(const std::vector<uint8_t> &payload)
    {
        connection.write(static_cast<uint8_t>(operation_target::client), static_cast<uint8_t>(response_operation::close_order), payload);
    }
}