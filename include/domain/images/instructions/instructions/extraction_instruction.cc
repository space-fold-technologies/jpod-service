#include <domain/images/instructions/extraction_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/import_resolver.h>
#include <domain/images/instructions/compression_errors.h>
#include <domain/images/payload.h>
#include <spdlog/spdlog.h>
#include <fstream>
#include <zip.h>
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
        zip_stat_t fs_stats = {};
        if (auto error = initialize(); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else if (zip_stat(archive_ptr, FILE_SYSTEM_ARCHIVE.c_str(), ZIP_STAT_NAME | ZIP_STAT_SIZE | ZIP_FL_UNCHANGED, &fs_stats) == -1)
        {
            listener.on_instruction_complete(identifier, fetch_error_code());
        }
        else if (zip_file_t *file = zip_fopen(archive_ptr, FILE_SYSTEM_ARCHIVE.c_str(), ZIP_FL_COMPRESSED); file == NULL)
        {
            listener.on_instruction_complete(identifier, fetch_error_code());
        }
        else
        {
            listener.on_instruction_initialized(identifier, name);
            std::ofstream archive_stream(image_archive, std::ios::binary | std::ios::app);
            frame.entry_name = identifier;
            frame.sub_entry_name = fmt::format("file system extraction {}", fs_stats.name);
            frame.percentage = 0;
            zip_int64_t bytes_read = 0;
            zip_int64_t current_read = 0;
            std::error_code error;
            do
            {
                if (bytes_read = zip_fread(file, buffer.data(), FS_BUFFER_SIZE); bytes_read == -1)
                {
                    error = fetch_error_code();
                    break;
                }
                else if (bytes_read > 0)
                {
                    archive_stream.write(reinterpret_cast<const char *>(buffer.data()), bytes_read);
                    current_read += bytes_read;
                    frame.percentage = current_read / fs_stats.size;
                    listener.on_instruction_data_received(identifier, pack_progress_frame(frame));
                }

            } while (bytes_read > 0);
            zip_fclose(file);
            archive_stream.close();
            listener.on_instruction_complete(identifier, error);
        }
    }
    std::error_code extraction_instruction::initialize()
    {

        int error_no;
        std::error_code error;
        if (image_archive = resolver.generate_image_path(identifier, error); error)
        {
            return error;
        }
        else if (archive_ptr = zip_open(resolver.archive_file_path().c_str(), ZIP_RDONLY, &error_no); archive_ptr == NULL)
        {
            zip_error_t error;
            zip_error_init_with_code(&error, error_no);
            logger->error("cannot open zip archive {} {}", name, zip_error_strerror(&error));
            zip_error_fini(&error);
            return make_compression_error_code(error_no);
        }
        return {};
    }
    std::error_code extraction_instruction::fetch_error_code()
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
    extraction_instruction::~extraction_instruction()
    {
        buffer.clear();
        if (archive_ptr != NULL)
        {
            archive_ptr = nullptr;
        }
    }
}