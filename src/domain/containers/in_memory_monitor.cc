#include <domain/containers/in_memory_monitor.h>
#include <spdlog/spdlog.h>

namespace domain::containers
{
    in_memory_monitor::in_memory_monitor() : logger(spdlog::get("jpod"))
    {
    }
    void in_memory_monitor::on_operation_initialization()
    {
        logger->info("imm: initialization");
    }
    void in_memory_monitor::on_operation_output(const std::vector<uint8_t> &content)
    {
        logger->info("imm: output: {}", std::string(content.begin(), content.end()));
    }
    void in_memory_monitor::on_operation_failure(const std::error_code &error)
    {
        logger->info("imm: input: {}", error.message());
    }
    listener_category in_memory_monitor::type()
    {
        return listener_category::observer;
    }
    void in_memory_monitor::clear()
    {
        // no need to implement it
    }
    in_memory_monitor::~in_memory_monitor()
    {
    }

}