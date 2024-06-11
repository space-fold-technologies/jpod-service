#ifndef __DAEMON_DOMAIN_IMAGES_BUILD_COMMAND_HANDLER__
#define __DAEMON_DOMAIN_IMAGES_BUILD_COMMAND_HANDLER__

#include <core/commands/command_handler.h>
#include <domain/images/instructions/instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <functional>
#include <filesystem>
#include <memory>
#include <map>
#include <deque>

namespace spdlog
{
    class logger;
};

namespace asio
{
    class io_context;
};

namespace domain::images::instructions
{
    class directory_resolver;
};

namespace core::oci
{
    class oci_client;
};

using namespace domain::images::instructions;
namespace fs = std::filesystem;
namespace domain::images
{
    class image_repository;
    class build_order;
    using oci_client_provider = std::function<std::unique_ptr<core::oci::oci_client>()>;
    using task = std::shared_ptr<instruction>;

    class build_handler : public core::commands::command_handler, public instruction_listener
    {
    public:
        explicit build_handler(
            core::connections::connection &connection,
            std::shared_ptr<image_repository> repository,
            oci_client_provider provider,
            asio::io_context &context);
        virtual ~build_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;
        void on_instruction_initialized(std::string id, std::string name) override;
        void on_instruction_data_received(std::string id, const std::vector<uint8_t> &content) override;
        void on_instruction_complete(std::string id, std::error_code err) override;

    private:
        void setup_stages(const build_order &order);
        void resolve_stage_name(const std::string &identifer, const std::string &order);
        void run_stages();
        void run_stage(const std::string &identifier);
        task create_download_instruction(const std::string &stage_identifier, const std::string &order);
        task create_mount_instruction(const std::string &stage_identifier, const std::string &order);
        task create_copy_instruction(const std::string &stage_identifier, const std::string &order);
        task create_work_dir_instruction(const std::string &stage_identifier, const std::string &order);
        task create_run_instruction(const std::string &stage_identifier, const std::string &order);
        task create_unmount_instruction(const std::string &stage_identifier, const std::string &order);
        task create_archive_instruction(const std::string &stage_identifier);
        task create_registration_instruction(const std::string &stage_identifier, const build_order &order, const std::string &parent_order);
        task create_cleanup_instruction(const std::string &stage_identifier, std::vector<std::string> stage_identifiers);

    private:
        asio::io_context &context;
        oci_client_provider provider;
        std::map<std::string, std::deque<task>> stages;
        std::map<std::string, fs::path> current_stage_work_directories;
        std::map<std::string, std::string> stage_names;
        std::unique_ptr<directory_resolver> resolver;
        std::shared_ptr<image_repository> repository;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_BUILD_COMMAND_HANDLER__