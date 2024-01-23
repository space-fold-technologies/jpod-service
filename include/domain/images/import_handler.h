#ifndef __DAEMON_DOMAIN_IMAGES_IMPORT_COMMAND_HANDLER__
#define __DAEMON_DOMAIN_IMAGES_IMPORT_COMMAND_HANDLER__

#include <core/commands/command_handler.h>
#include <domain/images/instructions/instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/import_resolver.h>
#include <filesystem>
#include <memory>
#include <map>
#include <deque>

namespace spdlog
{
    class logger;
};
using namespace domain::images::instructions;
namespace fs = std::filesystem;
namespace domain::images
{
    class image_repository;
    struct import_order;
    typedef std::shared_ptr<instruction> task;
    class import_handler : public core::commands::command_handler, public instruction_listener, public import_resolver
    {
    public:
        import_handler(
            core::connections::connection &connection,
            std::shared_ptr<image_repository> repository);
        virtual ~import_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;
        void on_instruction_initialized(std::string id, std::string name) override;
        void on_instruction_data_received(std::string id, const std::vector<uint8_t> &content) override;
        void on_instruction_complete(std::string id, std::error_code err) override;
        fs::path archive_file_path() override;
        fs::path generate_image_path(const std::string &identifier, std::error_code &error) override;

    private:
        void run_steps();

    private:
        std::string identifier;
        fs::path local_file_path;
        fs::path image_folder;
        std::deque<task> tasks;
        std::shared_ptr<image_repository> repository;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_IMPORT_COMMAND_HANDLER__
