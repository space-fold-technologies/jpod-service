#ifndef __JPOD_SERVICE_IMAGES_STAGE__
#define __JPOD_SERVICE_IMAGES_STAGE__

#include <images/instruction.h>
#include <memory>
#include <deque>
#include <optional>
#include <functional>
#include <tl/expected.hpp>
#include <system_error>
namespace asio
{
    class io_context;
};

namespace spdlog
{
    class logger;
};
namespace core::networks::http
{
    class Client;
};

namespace core::filesystems
{
    class FileSystemHandler;
};

using namespace core::networks::http;
using namespace core::filesystems;

namespace images
{
    class Instruction;
    class ImageRepository;
    struct StageDetails;
    typedef std::function<tl::expected<std::string, std::error_code>(const std::string&)> image_stage_resolver;
    class Stage : public InstructionListener
    {
    public:
        Stage(asio::io_context &context, const std::string &id, const std::string &local_directory, const StageDetails &details);
        virtual ~Stage();

        void initialize(
            std::shared_ptr<Client> client,
            std::shared_ptr<ImageRepository> repository,
            std::shared_ptr<FileSystemHandler> handler,
            image_stage_resolver resolver);
        void run();
        void on_instruction_runner_initialized(std::string id) override;
        void on_instruction_runner_data_received(std::string id, const std::vector<uint8_t> &content) override;
        void on_instruction_runner_completion(std::string id, const std::error_code &err) override;
        std::string image_local_directory();

    private:
        std::shared_ptr<Instruction> from_instruction(
            const std::string &order,
            std::shared_ptr<Client> client,
            std::shared_ptr<ImageRepository> repository,
            std::shared_ptr<FileSystemHandler> handler);
        std::shared_ptr<Instruction> run_instruction(const std::string &order);
        std::shared_ptr<Instruction> copy_instruction(const std::string &order, image_stage_resolver resolver);
        std::shared_ptr<Instruction> work_dir_instruction(const std::string &order);

    private:
        asio::io_context &context;
        const std::string &id;
        const std::string &local_directory;
        const StageDetails &details;
        std::deque<std::shared_ptr<Instruction>> instructions;
        std::string current_image_identifier;
        std::string current_work_directory;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_IMAGES_STAGE__