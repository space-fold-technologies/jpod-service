#include <core/commands/command_handler.h>
#include <core/commands/internal_contracts.h>
#include <core/connections/connection.h>
#include <core/connections/frame.h>
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
        auto payload = encode_frame(operation_target::client, response_operation::failure, pack_error_payload(error));
        connection.write(payload);
    }
    void command_handler::send_error(const std::string &err)
    {
        auto error = error_payload{err};
        auto payload = encode_frame(operation_target::client, response_operation::failure, pack_error_payload(error));
        connection.write(payload);
    }
    void command_handler::send_frame(const std::vector<uint8_t> &payload)
    {
        auto content = encode_frame(operation_target::client, response_operation::frame, payload);
        connection.write(content);
    }
    void command_handler::send_success(const std::string &message)
    {
        auto payload = success_payload{message};
        auto success = pack_success_payload(payload);
        auto content = encode_frame(operation_target::client, response_operation::success, success);
        connection.write(content);
    }
    void command_handler::send_progress(const std::string &operation, const std::vector<uint8_t> &data)
    {
        progress_payload order{operation, data};
        auto progress = pack_progress_payload(order);
        auto content = encode_frame(operation_target::client, response_operation::progress, progress);
        connection.write(content);
    }
    void command_handler::send_close(const std::vector<uint8_t> &payload)
    {
    }
}