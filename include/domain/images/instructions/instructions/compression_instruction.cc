#include <domain/images/instructions/compression_instruction.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/mappings.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fmt/format.h>
#include <zip.h>
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
            for (const auto &entry : std::filesystem::recursive_directory_iterator(image_filesystem_directory))
            {
                if (entry.is_regular_file())
                {
                    fs::path relative_path = fs::relative(entry.path(), image_filesystem_directory);
                    if (zip_source_t *source = zip_source_file(archive_ptr, entry.path().c_str(), 0, ZIP_LENGTH_TO_END); source == nullptr)
                    {
                        listener.on_instruction_complete(identifier, fetch_error_code());
                        return;
                    }
                    else
                    {
                        if (auto index = zip_file_add(archive_ptr, (relative_path / "").c_str(), source, ZIP_FL_OVERWRITE); index == OPERATION_FAILED)
                        {
                            listener.on_instruction_complete(identifier, fetch_error_code());
                            zip_source_free(source);
                            return;
                        }
                        else if (zip_set_file_compression(archive_ptr, index, ZIP_CM_ZSTD, 9) == OPERATION_FAILED)
                        {
                            listener.on_instruction_complete(identifier, fetch_error_code());
                            zip_source_free(source);
                            return;
                        }
                        else
                        {
                            zip_file_set_mtime(archive_ptr, index, time(nullptr), 0);
                        }
                    }
                }
                else if (entry.is_directory())
                {
                    const auto &file_path = entry.path();
                    const auto &relative_path = std::filesystem::relative(file_path, image_filesystem_directory);
                    if (zip_dir_add(archive_ptr, relative_path.c_str(), ZIP_FL_ENC_UTF_8) == OPERATION_FAILED)
                    {
                        listener.on_instruction_complete(identifier, fetch_error_code());
                        return;
                    }
                }
            }
            if (zip_close(archive_ptr) == OPERATION_FAILED)
            {
                listener.on_instruction_complete(identifier, fetch_error_code());
                zip_discard(archive_ptr);
            } else {
                listener.on_instruction_complete(identifier, {});
            }
        }
    }
    std::error_code compression_instruction::initialize()
    {
        int error_no;
        std::error_code error;
        if (image_filesystem_directory = resolver.destination_path(identifier, error); error)
        {
            return error;
        }
        else if (image_folder = resolver.generate_image_path(identifier, error); error)
        {
            return error;
        }
        else if (archive_ptr = zip_open(fs::path(image_folder / fs::path("fs.zip")).c_str(), ZIP_CREATE | ZIP_EXCL | ZIP_CHECKCONS, &error_no); archive_ptr == NULL)
        {
            zip_error_t error;
            zip_error_init_with_code(&error, error_no);
            logger->error("cannot open zip archive {} {}", name, zip_error_strerror(&error));
            zip_error_fini(&error);
            return make_compression_error_code(error_no);
        }
        else
        {
            zip_register_progress_callback_with_state(
                archive_ptr,
                PROGRESSION_PRECISION,
                &compression_instruction::on_progress_update,
                nullptr,
                this);
            return error;
        }
    }
    std::error_code compression_instruction::fetch_error_code()
    {
        std::error_code err;
        zip_error_t *error = zip_get_error(archive_ptr);

        if (zip_error_code_zip(error) != ZIP_ER_OK)
        {
            err = make_compression_error_code(error->zip_err);
        }
        else
        {
            err = std::make_error_code(static_cast<std::errc>(zip_error_code_system(error)));
        }
        zip_error_fini(error);
        logger->error("cannot operate on archive {} {}", name, err.message());
        return err;
    }
    void compression_instruction::on_progress_update(zip_t *zip_ctx, double progress, void *user_data)
    {
        auto self = static_cast<compression_instruction *>(user_data);
        self->frame.percentage = progress;
        self->listener.on_instruction_data_received(self->identifier, pack_progress_frame(self->frame));
    }

    compression_instruction::~compression_instruction()
    {
        if(archive_ptr != NULL) 
        {
            archive_ptr = nullptr;
        }
    }
}