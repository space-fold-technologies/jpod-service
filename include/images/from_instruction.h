#ifndef __JPOD_SERVICE_IMAGES_FROM_INSTRUCTION__
#define __JPOD_SERVICE_IMAGES_FROM_INSTRUCTION__

#include <images/instruction.h>
#include <core/networks/http/download_components.h>
#include <asio/io_context.hpp>
#include <memory>
#include <deque>

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
    class ImageRepository;
    class AccessDetails;
    class ImageMetaData;
    class FromInstruction : public Instruction, public Destination, public std::enable_shared_from_this<FromInstruction>
    {
    public:
        explicit FromInstruction(asio::io_context &context,
                        const std::string &id,
                        const std::string &image_identifier,
                        const std::string &source,
                        std::shared_ptr<Client> client,
                        std::shared_ptr<ImageRepository> repository,
                        std::shared_ptr<FileSystemHandler> handler,
                        InstructionListener &listener);
        virtual ~FromInstruction();
        std::error_code parse() override;
        void execute() override;
        bool is_valid() override;
        size_t chunk_size() const override;
        size_t write(const std::vector<uint8_t> &data) override;

    private:
        void fetch_image_details(const AccessDetails &details, const std::string &name, const std::string &version);
        void download_image_filesystem(const AccessDetails &details, const ImageMetaData &metadata);

    private:
        asio::io_context &context;
        const std::string &id;
        const std::string &image_identifier;
        const std::string &source;
        std::shared_ptr<Client> client;
        std::shared_ptr<ImageRepository> repository;
        std::shared_ptr<FileSystemHandler> handler;
        std::string file_path;
        std::size_t buffer_size;
        InstructionListener &listener;
        bool exists;
        std::deque<AccessDetails> registries;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_IMAGES_FROM_INSTRUCTION__