#include <images/from_instruction.h>
#include <images/image_repository.h>
#include <core/networks/http/client.h>
#include <core/networks/http/request.h>
#include <core/networks/http/request_builder.h>
#include <core/filesystems/handler.h>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <sys/utsname.h>
#include <spdlog/spdlog.h>
#include <vector>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace images
{
    FromInstruction::FromInstruction(asio::io_context &context,
                                     const std::string &id,
                                     const std::string &image_identifier,
                                     const std::string &source,
                                     std::shared_ptr<core::networks::http::Client> client,
                                     std::shared_ptr<ImageRepository> repository,
                                     std::shared_ptr<FileSystemHandler> handler,
                                     InstructionListener &listener) : context(context),
                                                                      id(id),
                                                                      image_identifier(image_identifier),
                                                                      source(source),
                                                                      client(client),
                                                                      repository(repository),
                                                                      handler(handler),
                                                                      listener(listener),
                                                                      logger(spdlog::get("jpod"))
    {
    }
    std::error_code FromInstruction::parse()
    {
        if (source.empty())
        {
            return std::make_error_code(std::errc::io_error);
        }
        auto parts = source | ranges::view::split(':') | ranges::to<std::vector<std::string>>();
        if (parts.size() < 1)
        {
            return std::make_error_code(std::errc::io_error);
        }
        auto name = parts.at(0);
        auto version = parts.size() > 1 ? parts.at(1) : "latest";
        this->exists = repository->exists(name, version);
        // also check to see if we have the
        return {};
    }
    void FromInstruction::execute()
    {
        registries = repository->active_registries() | ranges::to<std::deque<AccessDetails>>();
    }
    void FromInstruction::fetch_image_details(const AccessDetails &details, const std::string &name, const std::string &version)
    {
        auto query = ImageQuery{};
        query.name = name;
        query.version = version;
        utsname machine_details;
        if (uname(&machine_details) != 0)
        {
            listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
        }
        else
        {
            query.architecture = std::string(machine_details.machine);
            auto body = msgpack::pack(query);
            auto request = core::networks::http::Request::builder()
                               .address(fmt::format("{}/images/fs/look-up", details.uri))
                               .add_header("Content-Type", "application/x-msgpack")
                               .add_header("Content-Length", fmt::format("{}", body.size()))
                               .add_header("Authorization", fmt::format("Bearer {}", details.token))
                               .body(body)
                               .post()
                               .build();
            if (request.has_value())
            {
                client->execute(
                    request.value(),
                    [this, &details](const core::networks::http::Response &response)
                    {
                        logger->info("STATUS CODE: {}", response.status_code);
                        logger->info("CONTENT-LENGTH: {}", response.content_length());
                        logger->info("CONTENT-TYPE: {}", response.content_type());
                        if (response.has_body() && response.content_type() == "application/x-msgpack")
                        {
                            // so this means we can move on to the next step
                            auto payload = msgpack::unpack<ImageMetaData>(response.data);
                            download_image_filesystem(details, payload);
                        }
                        else
                        {
                            listener.on_instruction_runner_completion(this->id, response.err);
                        }
                    });
            }
        }
    }
    void FromInstruction::download_image_filesystem(const AccessDetails &details, const ImageMetaData &metadata)
    {
        file_path = fmt::format("/tmp/{}.fs.gz", metadata.id);
        client->download(
            fmt::format("{}/images/fs/{}.fs.gz", details.uri, metadata.id),
            shared_from_this(),
            [this, metadata](const core::networks::http::Status &status)
            {
                BuildUpdate update{};
                update.id = metadata.id;
                if (!status.err)
                {
                    Progress progress{metadata.name, status.start, status.end, status.total, status.unit, status.complete};
                    update.type = "progress";
                    update.data = msgpack::pack(progress);
                    logger->info("progress : {}/{}", status.current, status.total);
                    listener.on_instruction_runner_data_received(this->id, msgpack::pack(update));
                    if (status.complete)
                    {
                        // unpack to destination
                        auto result = handler->extract_snapshot(file_path, this->image_identifier, VolumeType::IMAGE);
                        if (result.has_value())
                        {
                            std::error_code err;
                            // remove archive
                            fs::remove(fs::path(file_path), err);
                            listener.on_instruction_runner_completion(this->id, err);
                        }
                        else
                        {
                            listener.on_instruction_runner_completion(this->id, result.error());
                        }
                    }
                }
                else
                {
                    logger->error("download failure : {}", status.err.message());
                    listener.on_instruction_runner_completion(this->id, status.err);
                }
            });
    }
    bool FromInstruction::is_valid()
    {
        if (!fs::exists(file_path))
        {
            auto path = fs::path(file_path);
            fs::create_directories(path.parent_path());
            fs::permissions(path.parent_path(),
                            fs::perms::owner_all | fs::perms::group_all,
                            fs::perm_options::add);
            return true;
        }
        if (!fs::is_regular_file(file_path))
        {
            return false;
        }
        auto permissions = fs::status(file_path).permissions();
        return fs::perms::none != (fs::perms::owner_write & permissions) || fs::perms::none != (fs::perms::group_write & permissions);
    }
    size_t FromInstruction::chunk_size() const
    {
        return buffer_size;
    }
    size_t FromInstruction::write(const std::vector<uint8_t> &data)
    {
        logger->info("writing {} bytes to file", data.size());
        std::ofstream file(file_path, std::ios::binary | std::ios::app);
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
        return data.size();
    }
    FromInstruction::~FromInstruction()
    {
        registries.clear();
    }
}