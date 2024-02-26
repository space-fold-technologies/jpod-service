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
            frame.sub_entry_name = "fs.zip";
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
                archive_write_header(archive_ptr.get(), archive_entry_ptr.get());
                struct stat file_info;
                stat(entry.path().c_str(), &file_info);
                archive_entry_copy_stat(archive_entry_ptr.get(), &file_info);
                if (archive_entry_set_size(archive_entry_ptr.get(), fs::file_size(entry, error)); error)
                {
                    listener.on_instruction_complete(identifier, error);
                    return;
                }
                if (entry.is_regular_file())
                {
                    std::vector<char> buffer(8096);
                    // Copy here
                    std::ifstream file(entry, std::ios::binary | std::ios::ate);
                    if (!file.is_open())
                    {
                        listener.on_instruction_complete(identifier, std::make_error_code(std::errc::io_error));
                        return;
                    }
                    std::size_t length = 0;
                    do
                    {
                        file.read(buffer.data(), buffer.size());
                        length = file.gcount();
                        if (length > 0)
                        {
                            archive_write_data(archive_ptr.get(), buffer.data(), length);
                        }
                    } while (length > 0);
                    file.close();
                    archive_entry_set_mtime(archive_entry_ptr.get(), time(nullptr), 0L);
                }
                else if (entry.is_directory())
                {
                    // TODO: switch out all this other stuff since it's not relevant
                    const auto &file_path = entry.path();
                    const auto &relative_path = std::filesystem::relative(file_path, image_filesystem_directory);
                    archive_entry_set_filetype(archive_entry_ptr.get(), AE_IFDIR);
                    archive_write_header(archive_ptr.get(), archive_entry_ptr.get());
                }
                archive_entry_clear(archive_entry_ptr.get());
            }
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
        else if (archive_write_open_filename(archive_ptr.get(), fs::path(image_folder / fs::path("fs.tar.gz")).c_str()) != ARCHIVE_OK)
            // {
            //     zip_error_t error;
            //     zip_error_init_with_code(&error, error_no);
            //     logger->error("cannot open zip archive {} {}", name, zip_error_strerror(&error));
            //     zip_error_fini(&error);
            //     return make_compression_error_code(error_no);
            // }
            // else
            // {
            //     zip_register_progress_callback_with_state(
            //         archive_ptr,
            //         PROGRESSION_PRECISION,
            //         &compression_instruction::on_progress_update,
            //         nullptr,
            //         this);
            //     return error;
            // }
            return {};
    }
    std::error_code compression_instruction::fetch_error_code()
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
        // logger->error("cannot operate on archive {} {}", name, err.message());
        return err;
    }
    // void compression_instruction::on_progress_update(zip_t *zip_ctx, double progress, void *user_data)
    // {
    //     auto self = static_cast<compression_instruction *>(user_data);
    //     self->frame.percentage = progress;
    //     self->listener.on_instruction_data_received(self->identifier, pack_progress_frame(self->frame));
    // }

    compression_instruction::~compression_instruction()
    {
        if (archive_ptr != NULL)
        {
            archive_ptr = nullptr;
        }
    }
}