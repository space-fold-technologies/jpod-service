#include <domain/images/instructions/import_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/import_resolver.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/payload.h>
#include <domain/images/mappings.h>
#include <domain/images/repository.h>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <archive.h>
#include <archive_entry.h>

namespace fs = std::filesystem;
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
                                                                             logger(spdlog::get("jpod")) {}
    void import_instruction ::execute()
    {

        if (auto error = initialize(); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else
        {
            bool found = false;
            archive_entry *entry;
            int ec = 0;
            listener.on_instruction_initialized(identifier, this->name);
            do
            {
                ec = archive_read_next_header(archive_ptr.get(), &entry);
                if (ec != ARCHIVE_OK)
                {
                    if (ec == ARCHIVE_EOF)
                    {
                        listener.on_instruction_complete(identifier, std::make_error_code(std::errc::no_such_file_or_directory));
                        return;
                    }
                    listener.on_instruction_complete(identifier, make_compression_error_code(ec));
                    return;
                }
                const mode_t type = archive_entry_filetype(entry);
                char const *current_entry_name = archive_entry_pathname(entry);

                if ((S_ISREG(type)) && std::strcmp(current_entry_name, IMAGE_INFO.c_str()) == 0)
                {
                    std::size_t file_size = archive_entry_size(entry);
                    std::vector<uint8_t> buffer(file_size);
                    auto chunk_size = archive_read_data(archive_ptr.get(), buffer.data(), file_size);
                    if (chunk_size < 0)
                    {
                        logger->error("{}", archive_error_string(archive_ptr.get()));
                        listener.on_instruction_complete(identifier, make_compression_error_code(chunk_size));
                        return;
                    }
                    listener.on_instruction_data_received(identifier, pack_progress_frame(progress_frame{ "extracting image meta-data", 100.0f}));
                    auto details = unpack_import_details(buffer);
                    if (auto path = resolver.image_file_path(identifier, error); error)
                    {
                        listener.on_instruction_complete(identifier, error);
                    }
                    else if (auto image_size = fs::file_size(path, error); error)
                    {
                        listener.on_instruction_complete(identifier, error);
                    }
                    else
                    {
                        error = persist_image_details(details, image_size);
                        if (!error)
                        {
                            listener.on_instruction_data_received(identifier, pack_progress_frame(progress_frame{"persisted image details", 100.0f}));
                        }
                        listener.on_instruction_complete(identifier, error);
                    }
                    found = true;
                }
                archive_read_data_skip(archive_ptr.get());

            } while (!found && ec == ARCHIVE_OK);
        }
    }
    std::error_code import_instruction::persist_image_details(const import_details &details, std::size_t file_size)
    {
        image_details image;
        image.identifier = identifier;
        image.name = details.name;
        image.tag = details.tag;
        image.os = details.os;
        image.variant = details.variant;
        image.version = details.version;
        image.registry_path = "localhost";
        image.entry_point = details.entry_point;
        image.size = file_size;
        image.labels.insert(details.labels.begin(), details.labels.end());
        image.env_vars.insert(details.env_vars.begin(), details.env_vars.end());
        image.parameters.insert(details.parameters.begin(), details.parameters.end());
        for (const auto &mp : details.mount_points)
        {
            image.mount_points.push_back(std::move((mount_point{mp.filesystem, mp.folder, mp.options, mp.flags})));
        }
        return repository.save_image_details(image);
    }

    std::error_code import_instruction::initialize()
    {

        std::error_code error;
        archive_ptr = {
            archive_read_new(),
            [](archive *instance) -> void
            {
                archive_read_close(instance);
                archive_read_free(instance);
            }};
        archive_read_support_filter_all(archive_ptr.get());
        archive_read_support_format_tar(archive_ptr.get());
        if (auto ec = archive_read_open_filename(archive_ptr.get(), resolver.archive_file_path().c_str(), INFO_BUFFER_SIZE); ec != ARCHIVE_OK)
        {
            logger->error("{}", archive_error_string(archive_ptr.get()));
            return make_compression_error_code(ec);
        }
        return {};
    }
    import_instruction ::~import_instruction()
    {
        if (archive_ptr != nullptr)
        {
            archive_ptr.reset();
            archive_ptr = nullptr;
        }
    }
}