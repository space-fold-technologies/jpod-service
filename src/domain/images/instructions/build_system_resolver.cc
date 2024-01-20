#include <domain/images/instructions/build_system_resolver.h>
#include <domain/images/instructions/compression_errors.h>
#include <zip.h>
#include <iostream>
#include <fstream>
#include <array>
#include <spdlog/spdlog.h>

namespace domain::images::instructions
{
    build_system_resolver::build_system_resolver(
        std::string local_directory,
        const std::map<std::string, std::string> &stage_names) : local_directory(local_directory),
                                                                 stage_names(stage_names),
                                                                 logger(spdlog::get("jpod"))
    {
    }
    fs::path build_system_resolver::local_folder()
    {
        return fs::path(local_directory);
    }
    fs::path build_system_resolver::stage_path(const std::string &label, std::error_code &error)
    {
        if (auto position = stage_names.find(label); position != stage_names.end())
        {
            fs::path output_folder = temporary_folder / fs::path(position->second);
            if (fs::is_directory(output_folder) && fs::exists(output_folder))
            {
                return output_folder;
            }
            else if (!fs::create_directories(output_folder.parent_path(), error))
            {
                logger->error("FS GEN {STAGE-PATH-QUERY} ERR : {}", error.message());
            }
            return output_folder;
        }
        return fs::path("");
    }
    fs::path build_system_resolver::destination_path(const std::string &identifier, std::error_code &error)
    {
        // this is a temporary folder you will need to create
        fs::path output_folder = temporary_folder / fs::path(identifier);
        if (fs::is_directory(output_folder) && fs::exists(output_folder))
        {
            return output_folder;
        }
        else if (!fs::create_directories(output_folder.parent_path(), error))
        {
            logger->error("FS GEN {STAGE-PATH} ERR : {}", error.message());
        }
        return output_folder;
    }
    fs::path build_system_resolver::generate_image_path(const std::string &identifier, std::error_code &error)
    {
        // generate a folder in a pre-fixed path that has the ${identifier} as the final folder
        fs::path image_fs_archive = image_folder / fs::path(identifier) / fs::path("fs.zip");
        if (!fs::create_directories(image_fs_archive.parent_path(), error))
        {
            logger->error("FS GEN ERR : {}", error.message());
        }
        return image_fs_archive;
    }
    void build_system_resolver::extract_image(const std::string &identifier, const std::string &image_identifier, extraction_callback callback)
    {
        fs::path image_fs_archive = image_folder / fs::path(image_identifier) / fs::path("fs.zip");
        fs::path output_folder = temporary_folder / fs::path(identifier);
        std::error_code error;
        progress_frame frame{};
        frame.entry_name = identifier;
        frame.sub_entry_name = "extracting fs.zip";
        if (archive_ptr archive = initialize_archive(image_fs_archive, error); error)
        {
            callback(error, frame);
        }
        else
        {
            auto archive_entry_count = zip_get_num_entries(archive.get(), 0);
            for (auto archive_entry_index = 0; archive_entry_index < archive_entry_count; ++archive_entry_index)
            {
                if (const auto *entry_name = zip_get_name(archive.get(), archive_entry_index, 0); entry_name == nullptr)
                {
                    callback(make_compression_error_code(ZIP_ER_NOENT), frame);
                }
                else
                {
                    fs::path full_path = output_folder / fs::path(std::string(entry_name));
                    if (!fs::create_directories(full_path.parent_path(), error))
                    {
                        callback(error, frame);
                    }
                    else if (auto out_file = std::ofstream(full_path, std::ios::binary); !out_file)
                    {
                        callback(std::make_error_code(std::errc::io_error), frame);
                    }
                    else if (auto *input_file = zip_fopen(archive.get(), entry_name, 0))
                    {
                        callback(std::make_error_code(std::errc::io_error), frame);
                    }
                    else
                    {
                        std::array<char, BUFFER_SIZE> buffer;
                        zip_int64_t bytes_read = 0;
                        frame.sub_entry_name = std::string(entry_name);
                        do
                        {
                            bytes_read = zip_fread(input_file, buffer.data(), BUFFER_SIZE);
                            if (bytes_read > 0)
                            {
                                out_file.write(buffer.data(), bytes_read);
                            }
                        } while (bytes_read > 0);
                        zip_fclose(input_file);
                        out_file.close();
                        callback(error, frame);
                    }
                }
            }
        }
    }

    archive_ptr build_system_resolver::initialize_archive(const fs::path &image_fs_archive, std::error_code &error)
    {
        int error_no;
        // get the general folder path for the images ending in fs.zip
        if (auto ptr = zip_open(image_fs_archive.generic_string().c_str(), 0, &error_no); ptr == NULL)
        {
            zip_error_t zip_error;
            zip_error_init_with_code(&zip_error, error_no);
            logger->error("cannot open filesystem archive {}", zip_error_strerror(&zip_error));
            zip_error_fini(&zip_error);
            error = make_compression_error_code(error_no);
            return {nullptr, nullptr};
        }
        else
        {
            return {ptr, [](zip_t *ctx)
                    {
                        zip_close(ctx);
                    }};
        }
    }
    build_system_resolver::~build_system_resolver()
    {
    }
}