#include <domain/images/instructions/import_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/import_resolver.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/payload.h>
#include <domain/images/mappings.h>
#include <domain/images/repository.h>
#include <spdlog/spdlog.h>

namespace domain::images::instructions
{
    import_instruction::import_instruction(const std::string &identifier,
                                           image_repository &repository,
                                           import_resolver &resolver,
                                           instruction_listener &listener) : instruction("IMPORT", listener),
                                                                             identifier(identifier),
                                                                             repository(repository),
                                                                             resolver(resolver),
                                                                             archive_ptr(nullptr),
                                                                             chunk(INFO_BUFFER_SIZE),
                                                                             logger(spdlog::get("jpod")) {}
    void import_instruction ::execute()
    {

        if (auto error = initialize(); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        // else if (auto entry = fetch_details_entry(error); error)
        // {
        //     listener.on_instruction_complete(identifier, error);
        // }
        else
        {
            // listener.on_instruction_initialized(identifier, name);
            // if (auto result = extract_image_details(*entry, error); error)
            // {
            //     listener.on_instruction_complete(identifier, error);
            // }
            // else if (auto file_size = fs::file_size(resolver.image_file_path(identifier, error), error); error)
            // {
            //     listener.on_instruction_complete(identifier, error);
            // }
            // else
            // {
            //     image_details details;
            //     details.identifier = identifier;
            //     details.name = result->name;
            //     details.tag = result->tag;
            //     details.os = result->os;
            //     details.variant = result->variant;
            //     details.version = result->version;
            //     details.registry_path = "localhost";
            //     details.entry_point = result->entry_point;
            //     details.size = file_size;
            //     details.labels.insert(result->labels.begin(), result->labels.end());
            //     details.env_vars.insert(result->env_vars.begin(), result->env_vars.end());
            //     details.parameters.insert(result->parameters.begin(), result->parameters.end());
            //     for (const auto &mp : result->mount_points)
            //     {
            //         details.mount_points.push_back(mount_point{mp.filesystem, mp.folder, mp.options, mp.flags});
            //     }
            //     error = repository.save_image_details(details);
            //     listener.on_instruction_complete(identifier, error);
            // }
        }
    }
    // std::optional<import_details> import_instruction::extract_image_details(details_entry &entry, std::error_code &error)
    // {
        // buffer.reserve(entry.size);
        // zip_int64_t bytes_read = 0;
        // do
        // {
        //     if (bytes_read = zip_fread(entry.file, chunk.data(), INFO_BUFFER_SIZE); bytes_read == -1)
        //     {
        //         error = fetch_error_code();
        //         break;
        //     }
        //     else if (bytes_read > 0)
        //     {
        //         buffer.insert(buffer.end(), chunk.begin(), chunk.begin() + bytes_read);
        //     }
        //     logger->info("bytes transfered: {}", bytes_read);
        // } while (bytes_read > 0);
        // zip_fclose(entry.file);

        // if (error)
        // {
        //     return std::nullopt;
        // }
        // return unpack_import_details(buffer);
    //     return std::nullopt;
    // }
    std::error_code import_instruction::initialize()
    {

        int error_no;
        std::error_code error;
        // if (archive_ptr = zip_open(resolver.archive_file_path().c_str(), ZIP_RDONLY, &error_no); archive_ptr == NULL)
        // {
        //     zip_error_t error;
        //     zip_error_init_with_code(&error, error_no);
        //     logger->error("cannot open zip archive {} {}", name, zip_error_strerror(&error));
        //     zip_error_fini(&error);
        //     return make_compression_error_code(error_no);
        // }
        return {};
    }
    // std::optional<details_entry> import_instruction::fetch_details_entry(std::error_code &error)
    // {
    //     zip_stat_t fs_stab;
    //     for (zip_int64_t index = 0; index < zip_get_num_entries(archive_ptr, ZIP_FL_UNCHANGED); ++index)
    //     {
    //         if (zip_stat_index(archive_ptr, index, ZIP_STAT_NAME | ZIP_STAT_SIZE | ZIP_FL_UNCHANGED, &fs_stab) != -1)
    //         {
    //             if (strstr(fs_stab.name, IMAGE_INFO.c_str()) != NULL)
    //             {
    //                 return std::optional(details_entry{
    //                     zip_fopen_index(archive_ptr, index, ZIP_FL_UNCHANGED),
    //                     fs_stab.size,
    //                     std::string(fs_stab.name)});
    //             }
    //         }
    //         else
    //         {
    //             error = fetch_error_code();
    //         }
    //     }
    //     return std::nullopt;
    // }
    std::error_code import_instruction::fetch_error_code()
    {
        std::error_code err;
        // zip_error_t *error = zip_get_error(archive_ptr);

        // if (zip_error_code_zip(error) != ZIP_ER_OK)
        // {
        //     err = make_compression_error_code(error->zip_err);
        // }
        // else
        // {
        //     err = std::make_error_code(static_cast<std::errc>(zip_error_code_system(error)));
        // }
        // zip_error_fini(error);
        logger->error("cannot operate on archive {} {}", name, err.message());
        return err;
    }
    import_instruction ::~import_instruction() {}
}