#include <domain/containers/creation_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/orders.h>
#include <domain/images/helpers.h>
#include <domain/images/payload.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/instructions/errors.h>
#include <fstream>
#include <vector>
#include <spdlog/spdlog.h>
#include <sole.hpp>

namespace dmi = domain::images::instructions;
namespace domain::containers
{
    creation_handler::creation_handler(core::connections::connection &connection, const creation_configuration &configuration, std::shared_ptr<container_repository> repository) : command_handler(connection),
                                                                                                                                                                                   identifier(sole::uuid4().str()),
                                                                                                                                                                                   configuration(configuration),
                                                                                                                                                                                   repository(std::move(repository)),
                                                                                                                                                                                   logger(spdlog::get("jpod"))
    {
    }

    void creation_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_creation_order(payload);
        if (auto query = dmi::resolve_tagged_image_details(order.tagged_image); !query)
        {
            send_error(dmi::make_error_code(dmi::error_code::invalid_order_issued));
        }
        else if (auto details = repository->fetch_image_details(query->registry, query->name, query->tag); !details)
        {
            send_error(std::make_error_code(std::errc::resource_unavailable_try_again));
        }
        else if (auto error = initialize_decompression(details->identifier); error)
        {
            send_error(error);
        }
        else if (error = extract_filesystem(); error)
        {
            send_error(error);
        }
        else
        {
            container_properties properties;
            properties.identifier = identifier;
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
                send_error(error);
            }
            else
            {
                send_success("container created");
            }
        }
    }
    fs::path creation_handler::generate_container_folder(const std::string &identifier, std::error_code &error)
    {
        fs::path target = fs::path(configuration.containers_folder) / fs::path(identifier);
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
        // frame.entry_name = identifier;
        // frame.sub_entry_name = "extracting fs.zip";
        // std::error_code error;
        // auto archive_entry_count = zip_get_num_entries(archive_ptr, 0);
        // for (auto archive_entry_index = 0; archive_entry_index < archive_entry_count; ++archive_entry_index)
        // {
        //     if (const auto *entry_name = zip_get_name(archive_ptr, archive_entry_index, 0); entry_name == nullptr)
        //     {
        //         return fetch_decompression_error_code();
        //     }
        //     else
        //     {
        //         fs::path full_path = container_directory / fs::path(std::string(entry_name));
        //         if (!fs::create_directories(full_path.parent_path(), error))
        //         {
        //             return fetch_decompression_error_code();
        //         }
        //         else if (auto out_file = std::ofstream(full_path, std::ios::binary); !out_file)
        //         {
        //             return std::make_error_code(std::errc::io_error);
        //         }
        //         else if (auto *input_file = zip_fopen(archive_ptr, entry_name, 0); input_file == nullptr)
        //         {
        //             return std::make_error_code(std::errc::io_error);
        //         }
        //         else
        //         {
        //             std::vector<char> buffer(BUFFER_SIZE);
        //             zip_int64_t bytes_read = 0;
        //             do
        //             {
        //                 bytes_read = zip_fread(input_file, buffer.data(), BUFFER_SIZE);
        //                 if (bytes_read > 0)
        //                 {
        //                     out_file.write(buffer.data(), bytes_read);
        //                 }
        //             } while (bytes_read > 0);
        //             zip_fclose(input_file);
        //             out_file.close();
        //         }
        //     }
        // }
        return {};
    }
    fs::path creation_handler::fetch_image_archive(std::string &image_identifier, std::error_code &error)
    {
        fs::path target = fs::path(configuration.images_folder) / fs::path(image_identifier) / fs::path("fs.zip");
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
            return error;
        }
        // else if (auto image_fs_archive = fetch_image_archive(identifier, error); error)
        // {
        //     return error;
        // }
        // else if (archive_ptr = zip_open(image_fs_archive.c_str(), 0, &error_no); archive_ptr == NULL)
        // {
        //     zip_error_t error;
        //     zip_error_init_with_code(&error, error_no);
        //     logger->error("cannot open file system archive for :{} {}", image_identifier, zip_error_strerror(&error));
        //     zip_error_fini(&error);
        //     return dmi::make_compression_error_code(error_no);
        // }
        // else
        // {
        //     zip_register_progress_callback_with_state(
        //         archive_ptr,
        //         PROGRESSION_PRECISION,
        //         &creation_handler::on_progress_update,
        //         nullptr,
        //         this);
        //     return error;
        // }
        return {};
    }
    // std::error_code creation_handler::fetch_decompression_error_code()
    // {
    //     std::error_code err;
    //     zip_error_t *error = zip_get_error(archive_ptr);

    //     if (zip_error_code_zip(error) != ZIP_ER_OK)
    //     {
    //         err = dmi::make_compression_error_code(error->zip_err);
    //     }
    //     else
    //     {
    //         err = std::make_error_code(static_cast<std::errc>(zip_error_code_system(error)));
    //     }
    //     zip_error_fini(error);
    //     return err;
    // }
    // void creation_handler::on_progress_update(zip_t *zip_ctx, double progress, void *user_data)
    // {
    //     auto self = static_cast<creation_handler *>(user_data);
    //     self->frame.percentage = progress;
    //     self->send_progress("container-creation", pack_progress_frame(self->frame));
    // }
    void creation_handler::on_connection_closed(const std::error_code &error) {}

    creation_handler::~creation_handler()
    {
       
    }

}