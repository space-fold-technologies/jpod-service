#include <domain/images/instructions/compression_instruction.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/mappings.h>
#include <spdlog/spdlog.h>
#include <archive.h>
#include <archive_entry.h>
#include <filesystem>
#include <sys/stat.h>
#include <fmt/format.h>
#include <fstream>

namespace domain::images::instructions
{
    compression_instruction::compression_instruction(
        const std::string &identifier,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("ARCHIVE", listener),
                                          identifier(identifier),
                                          resolver(resolver),
                                          archive_ptr(nullptr),
                                          frame{},
                                          logger(spdlog::get("jpod"))
    {
    }

    void compression_instruction::execute()
    {
        if (auto error = initialize(); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else
        {
            listener.on_instruction_initialized(identifier, this->name);
            frame.entry_name = identifier;
            frame.sub_entry_name = "fs.tar.gz";
            archive_entry_ptr = {
                archive_entry_new(),
                [](archive_entry *instance) -> void
                {
                    archive_entry_free(instance);
                }};
            for (const auto &entry : std::filesystem::recursive_directory_iterator(image_filesystem_directory))
            {
                fs::path relative_path = fs::relative(entry.path(), image_filesystem_directory);
                archive_entry_set_pathname(archive_entry_ptr.get(), (relative_path / "").c_str());
                struct stat file_info;
                stat(entry.path().c_str(), &file_info);
                archive_entry_copy_stat(archive_entry_ptr.get(), &file_info);
                //frame.total = file_info.st_size;
                if (auto ec = archive_write_header(archive_ptr.get(), archive_entry_ptr.get()); ec != ARCHIVE_OK)
                {
                    logger->error("{}", archive_error_string(archive_ptr.get()));
                    listener.on_instruction_complete(identifier, make_compression_error_code(ec));
                    return;
                }
                if (entry.is_regular_file())
                {
                    std::vector<char> buffer(8096);
                    std::ifstream file(entry, std::ios::binary);
                    if (!file.is_open())
                    {
                        listener.on_instruction_complete(identifier, std::make_error_code(std::errc::io_error));
                        return;
                    }
                    std::size_t length = 0;
                    frame.feed = fmt::format("compressing {}", relative_path.generic_string());
                    do
                    {
                        file.read(buffer.data(), buffer.size());
                        length = file.gcount();
                        if (length > 0)
                        {
                            archive_write_data(archive_ptr.get(), buffer.data(), length);
                            listener.on_instruction_data_received(identifier, pack_progress_frame(frame));
                        }
                    } while (length > 0);
                    file.close();
                    archive_entry_set_mtime(archive_entry_ptr.get(), time(nullptr), 0L);
                }
                archive_entry_clear(archive_entry_ptr.get());
            }
            listener.on_instruction_complete(identifier, {});
        }
    }
    std::error_code compression_instruction::initialize()
    {
        int error_no;
        std::error_code error;
        archive_ptr = {
            archive_write_new(),
            [](archive *instance) -> void
            {
                archive_write_close(instance);
                archive_write_free(instance);
            }};
        archive_write_add_filter_gzip(archive_ptr.get()); // Add gzip compression
        archive_write_set_format_cpio(archive_ptr.get()); // Set cpio format
        archive_write_set_format_pax_restricted(archive_ptr.get());
        if (image_filesystem_directory = resolver.destination_path(identifier, error); error)
        {
            return error;
        }
        else if (image_folder = resolver.generate_image_path(identifier, error); error)
        {
            return error;
        }
        else if (auto ec = archive_write_open_filename(archive_ptr.get(), fs::path(image_folder / fs::path("fs.tar.gz")).c_str()); ec != ARCHIVE_OK)
        {
            logger->error("{}", archive_error_string(archive_ptr.get()));
            return make_compression_error_code(ec);
        }
        return {};
    }

    compression_instruction::~compression_instruction()
    {
        if (archive_ptr != NULL)
        {
            archive_ptr.reset();
            archive_ptr = nullptr;
        }
    }
}