#include <domain/images/instructions/download_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/helpers.h>
#include <domain/images/http/client.h>
#include <domain/images/http/request_builder.h>
#include <domain/images/http/contracts.h>
#include <domain/images/payload.h>
#include <domain/images/mappings.h>
#include <domain/images/repository.h>
#include <sys/utsname.h>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fstream>

namespace domain::images::instructions
{
    download_instruction::download_instruction(
        const std::string &identifier,
        const std::string &order,
        http::client &client,
        image_repository &repository,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("FROM", listener),
                                          identifier(identifier),
                                          order(order),
                                          client(client),
                                          repository(repository),
                                          resolver(resolver),
                                          logger(spdlog::get("jpod"))
    {
    }
    void download_instruction::execute()
    {
        if (auto result = resolve_tagged_image_details(order); !result.has_value())
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::invalid_order_issued));
        }
        else if (repository.has_image(result->registry, result->name, result->tag))
        {
            std::error_code error;
            if (fs::path stage_path = resolver.generate_image_path(identifier, error); error)
            {
                listener.on_instruction_complete(identifier, error);
            }
            else if (auto image_identifier = repository.fetch_image_identifier(result->registry, result->name, result->tag); image_identifier.has_value())
            {
                listener.on_instruction_initialized(identifier, name);
                resolver.extract_image(
                    identifier,
                    *image_identifier,
                    [this](std::error_code error, progress_frame &progress)
                    {
                        if (!error)
                        {
                            listener.on_instruction_data_received(identifier, pack_progress_frame(progress));
                            if (static_cast<int>(progress.percentage) == 100)
                            {
                                listener.on_instruction_complete(identifier, {});
                            }
                        }
                        else
                        {
                            listener.on_instruction_complete(identifier, error);
                        }
                    });
            }
        }
        else if (auto registry = result->registry == "local" ? repository.fetch_registry_by_name("local") : repository.fetch_registry_by_path(result->registry); registry.has_value())
        {
            fetch_image_details(*registry, result->name, result->tag);
        }
        else
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::no_registry_entries_found));
        }
    }

    void download_instruction::fetch_image_details(const registry_access_details &details, const std::string &name, const std::string &tag)
    {
        image_query query{};
        query.name = name;
        query.tag = tag;
        utsname machine_details;
        if (uname(&machine_details) != 0)
        {
            listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
        }
        else
        {
            std::error_code error;
            query.architecture = std::string(machine_details.machine);
            auto body = pack_image_query(query);
            auto request = http::request::builder()
                               .address(fmt::format("{}/images/fs/look-up", details.uri))
                               .add_header("Content-Type", "application/x-msgpack")
                               .add_header("Content-Length", fmt::format("{}", body.size()))
                               .add_header("Authorization", fmt::format("Bearer {}", details.token))
                               .body(body)
                               .post()
                               .build(error);
            if (error)
            {
                listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
            }
            else
            {
                client.execute(
                    request,
                    [this, &details](std::error_code err, const http::response &response)
                    {
                        logger->trace("STATUS CODE: {}", response.status_code);
                        logger->trace("CONTENT-LENGTH: {}", response.content_length());
                        logger->trace("CONTENT-TYPE: {}", response.content_type());
                        if (response.has_body() && response.content_type() == "application/x-msgpack")
                        {
                            // so this means we can move on to the next step
                            this->listener.on_instruction_initialized(this->identifier, this->name);
                            auto payload = unpack_image_details(response.data);
                            download_image_filesystem(details, payload);
                        }
                        else
                        {
                            listener.on_instruction_complete(this->identifier, err);
                        }
                    });
            }
        }
    }
    void download_instruction::download_image_filesystem(const registry_access_details &details, const image_meta &meta)
    {
        std::error_code err;
        auto uri = fmt::format("{}/{}", details.uri, meta.identifier);
        if (fs::path archive_destination = resolver.generate_image_path(meta.identifier, err); err)
        {
            listener.on_instruction_complete(identifier, err);
        }
        else
        {
            image_archive = archive_destination / fs::path("fs.zip");
            std::map<std::string, std::string> headers;
            headers.emplace("Authorization", fmt::format("Bearer {}", details.token));
            client.download(
                uri,
                headers,
                shared_from_this(),
                [this, &details, &meta](std::error_code error, http::download_status status)
                {
                    if (error)
                    {
                        listener.on_instruction_complete(identifier, error);
                    }
                    else
                    {
                        frame.percentage = (status.current * 100) / status.total;
                        // we will need to pack a progress frame for this
                        listener.on_instruction_data_received(identifier, pack_progress_frame(frame));
                        if (status.complete)
                        {
                            extract_image_filesystem(details, meta);
                        }
                    }
                });
        }
    }
    void download_instruction::extract_image_filesystem(const registry_access_details &details, const image_meta &meta)
    {
        resolver.extract_image(
            identifier,
            meta.identifier,
            [this, &details, &meta](std::error_code error, progress_frame &progress)
            {
                if (error)
                {
                    listener.on_instruction_complete(identifier, error);
                }
                else
                {
                    listener.on_instruction_data_received(identifier, pack_progress_frame(progress));
                    if (static_cast<int>(progress.percentage) == 100)
                    {
                        save_image_details(meta);
                    }
                }
            });
    }
    void download_instruction::save_image_details(const image_meta &meta)
    {

        image_details details{};
        details.identifier = meta.identifier;
        details.registry_path = meta.repository;
        details.name = meta.name;
        details.tag = meta.tag;
        details.size = meta.size;
        details.os = meta.os;
        details.variant = meta.variant;
        details.version = meta.version;
        auto transformer = [](const auto &p) -> domain::images::mount_point
        {
            return domain::images::mount_point{p.filesystem, p.folder, p.options, p.flags};
        };
        details.mount_points = meta.mount_points | ranges::views::transform(transformer) | ranges::to<std::vector<domain::images::mount_point>>();

        if (std::error_code error = repository.save_image_details(details); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else
        {
            listener.on_instruction_complete(identifier, {});
        }
    }
    bool download_instruction::is_valid()
    {
        if (!fs::exists(image_archive))
        {
            auto path = fs::path(image_archive);
            fs::create_directories(path.parent_path());
            fs::permissions(path.parent_path(),
                            fs::perms::owner_all | fs::perms::group_all,
                            fs::perm_options::add);
            return true;
        }
        if (!fs::is_regular_file(image_archive))
        {
            return false;
        }
        auto permissions = fs::status(image_archive).permissions();
        return fs::perms::none != (fs::perms::owner_write & permissions) || fs::perms::none != (fs::perms::group_write & permissions);
    }
    std::size_t download_instruction::chunk_size() const
    {
        return DOWNLOAD_BUFFER_SIZE;
    }
    std::size_t download_instruction::write(const std::vector<uint8_t> &data)
    {
        logger->trace("writing {} bytes to file", data.size());
        std::ofstream file(image_archive, std::ios::binary | std::ios::app);
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
        return data.size();
    }
    download_instruction::~download_instruction()
    {
    }
}