#include <domain/images/instructions/extraction_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/import_resolver.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/payload.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>

namespace domain::images::instructions
{
    extraction_instruction::extraction_instruction(const std::string &identifier,
                                                   import_resolver &resolver,
                                                   instruction_listener &listener) : instruction("EXTRACTION", listener),
                                                                                     identifier(identifier),
                                                                                     resolver(resolver),
                                                                                     archive_ptr(nullptr),
                                                                                     frame{},
                                                                                     buffer(FS_BUFFER_SIZE),
                                                                                     logger(spdlog::get("jpod"))
    {
    }
    void extraction_instruction::execute()
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
                if ((S_ISREG(type)) && std::strcmp(current_entry_name, FILE_SYSTEM_ARCHIVE.c_str()) == 0)
                {
                    std::size_t chunk_size = 0L;
                    std::size_t current_read = 0L;
                    std::size_t total_size = archive_entry_size(entry);
                    
                    logger->info("writing to : {}", image_archive.generic_string());
                    std::ofstream archive_stream(image_archive, std::ios::binary | std::ios::app);
                    frame.entry_name = identifier;
                    frame.sub_entry_name = fmt::format("file system extraction {}", current_entry_name);
                    do
                    {
                        chunk_size = archive_read_data(archive_ptr.get(), buffer.data(), FS_BUFFER_SIZE);
                        if (chunk_size < 0)
                        {
                            listener.on_instruction_complete(identifier, make_compression_error_code(chunk_size));
                            return;
                        }
                        else if (chunk_size > 0)
                        {
                            archive_stream.write(reinterpret_cast<const char *>(buffer.data()), chunk_size);
                            current_read += chunk_size;
                            frame.percentage = current_read / total_size;
                            listener.on_instruction_data_received(identifier, pack_progress_frame(frame));
                        }

                    } while (chunk_size > 0);
                    found = true;
                    archive_stream.close();
                }
                archive_read_data_skip(archive_ptr.get());
            } while (!found && ec == ARCHIVE_OK);
            listener.on_instruction_complete(identifier, {});
        }
    }
    std::error_code extraction_instruction::initialize()
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

        if (image_archive = resolver.generate_image_path(identifier, error); error)
        {
            return error;
        }
        else if (auto ec = archive_read_open_filename(archive_ptr.get(), resolver.archive_file_path().c_str(), FS_BUFFER_SIZE); ec != ARCHIVE_OK)
        {
            logger->error("{}", archive_error_string(archive_ptr.get()));
            return make_compression_error_code(ec);
        }
        return {};
    }

    extraction_instruction::~extraction_instruction()
    {
        buffer.clear();
        if (archive_ptr != nullptr)
        {
            archive_ptr.reset();
            archive_ptr = nullptr;
        }
    }
}