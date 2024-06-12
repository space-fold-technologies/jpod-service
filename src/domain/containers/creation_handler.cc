#include <domain/containers/creation_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/orders.h>
#include <domain/images/helpers.h>
#include <domain/images/payload.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/instructions/errors.h>
#include <archive.h>
#include <archive_entry.h>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <sole.hpp>

namespace dmi = domain::images::instructions;
namespace domain::containers
{
    creation_handler::creation_handler(
        core::connections::connection &connection,
        const fs::path &containers_folder,
        const fs::path &images_folder,
        std::shared_ptr<container_repository> repository) : command_handler(connection),
                                                            identifier(sole::uuid4().str()),
                                                            containers_folder(containers_folder),
                                                            images_folder(images_folder),
                                                            repository(std::move(repository)),
                                                            frame{},
                                                            logger(spdlog::get("jpod"))
    {
    }

    void creation_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_creation_order(payload);
        if (repository->exists(order.name))
        {
            send_error(fmt::format("a container with the name: {} exists", order.name));
        }
        else if (auto query = dmi::resolve_tagged_image_details(order.tagged_image); !query)
        {
            send_error(dmi::make_error_code(dmi::error_code::invalid_order_issued));
        }
        else if (auto details = repository->fetch_image_details(query->registry, query->repository, query->tag); !details)
        {
            send_error(std::make_error_code(std::errc::resource_unavailable_try_again));
        }
        else if (auto error = initialize_decompression(details->identifier); error)
        {
            send_error(fmt::format("decompression of image failed: {}", error.message()));
        }
        else if (error = extract_filesystem(); error)
        {
            send_error(fmt::format("file system extraction failed: {}", error.message()));
        }
        else
        {
            container_properties properties;
            properties.identifier = identifier;
            properties.os = details->os;
            properties.name = order.name;
            properties.image_identifier = details->identifier;
            properties.parameters.insert(details->parameters.begin(), details->parameters.end());
            properties.port_map.insert(order.port_map.begin(), order.port_map.end());
            properties.env_vars.insert(details->env_vars.begin(), details->env_vars.end());
            properties.env_vars.insert(order.env_vars.begin(), order.env_vars.end());
            properties.entry_point = details->entry_point;
            properties.network_properties = order.network_properties;
            if (auto error = repository->save(properties); error)
            {
                send_error(fmt::format("container creation failed: {}", error.message()));
            }
            else
            {
                send_success(identifier); // send the container ID over this call as the final response
            }
        }
    }
    fs::path creation_handler::generate_container_folder(const std::string &identifier, std::error_code &error)
    {
        fs::path target = containers_folder / fs::path(identifier);
        if (!fs::exists(target))
        {
            if (!fs::create_directories(target, error))
            {
                return "";
            }
        }
        return target;
    }
    std::error_code creation_handler::extract_filesystem()
    {
        std::error_code error;
        archive_entry *entry;
        while (archive_read_next_header(input.get(), &entry) == ARCHIVE_OK)
        {
            const char *entry_name = archive_entry_pathname(entry);
            const mode_t type = archive_entry_filetype(entry);
            fs::path full_path = container_directory / fs::path(std::string(entry_name));
            archive_entry_set_pathname(entry, full_path.generic_string().c_str());
            if (auto ec = archive_write_header(output.get(), entry); ec != ARCHIVE_OK)
            {
                logger->error("{}", archive_error_string(output.get()));
                return dmi::make_compression_error_code(ec);
            }
            else if (archive_entry_size(entry) > 0)
            {
                auto entry_size = archive_entry_size(entry);
                if (error = copy_entry(input.get(), output.get()); error)
                {
                    return error;
                }
                frame.feed = fmt::format("unpacked: {}", std::string(entry_name));
                send_progress(pack_progress_frame(frame));
            }
        }
        return {};
    }
    fs::path creation_handler::fetch_image_archive(std::string &image_identifier, std::error_code &error)
    {
        fs::path target = images_folder / fs::path(image_identifier) / fs::path("fs.tar.gz");
        if (!fs::exists(target, error))
        {
            return {};
        }
        return target;
    }
    std::error_code creation_handler::initialize_decompression(std::string &image_identifier)
    {
        int error_no;
        std::error_code error;
        if (container_directory = generate_container_folder(identifier, error); error)
        {
            logger->error("failed to create container folder");
            return error;
        }
        else if (auto image_fs_archive = fetch_image_archive(image_identifier, error); error)
        {
            return error;
        }
        else
        {
            input = {
                archive_read_new(),
                [](archive *instance) -> void
                {
                    archive_read_close(instance);
                    archive_read_free(instance);
                }};
            archive_read_support_filter_all(input.get());
            archive_read_support_format_tar(input.get());
            if (auto ec = archive_read_open_filename(input.get(), image_fs_archive.generic_string().c_str(), BUFFER_SIZE); ec != ARCHIVE_OK)
            {
                logger->error("{}", archive_error_string(input.get()));
                return dmi::make_compression_error_code(ec);
            }
            output = {
                archive_write_disk_new(),
                [](archive *instance) -> void
                {
                    archive_write_close(instance);
                    archive_write_free(instance);
                }};

            int flags = ARCHIVE_EXTRACT_TIME;
            flags |= ARCHIVE_EXTRACT_PERM;
            flags |= ARCHIVE_EXTRACT_ACL;
            flags |= ARCHIVE_EXTRACT_FFLAGS;
            archive_write_disk_set_options(output.get(), flags);
            archive_write_disk_set_standard_lookup(output.get());
            return {};
        }
    }
    std::error_code creation_handler::copy_entry(struct archive *in, struct archive *out)
    {
        int ec;
        const void *buffer;
        std::size_t size;
        la_int64_t offset;
        while ((ec = archive_read_data_block(in, &buffer, &size, &offset)) == ARCHIVE_OK)
        {
            if (ec = archive_write_data_block(out, buffer, size, offset); ec != ARCHIVE_OK)
            {
                logger->error("{}", archive_error_string(out));
                return dmi::make_compression_error_code(ec);
            }
        }
        if (ec = archive_write_finish_entry(out); ec != ARCHIVE_OK)
        {
            logger->error("{}", archive_error_string(out));
            return dmi::make_compression_error_code(ec);
        }
        return {};
    }
    void creation_handler::on_connection_closed(const std::error_code &error) {}

    creation_handler::~creation_handler()
    {
    }
}