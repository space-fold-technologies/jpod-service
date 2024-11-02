#include <domain/images/instructions/build_system_resolver.h>
#include <domain/images/instructions/compression_errors.h>
#include <iostream>
#include <fstream>
#include <array>
#include <spdlog/spdlog.h>
#include <archive.h>
#include <archive_entry.h>

namespace domain::images::instructions
{
    build_system_resolver::build_system_resolver(
        std::string local_directory,
        const std::map<std::string, std::string> &stage_names) : local_directory(local_directory),
                                                                 stage_names(stage_names),
                                                                 extensions{{"tar", "tar archives"}, {"tar.gz", "gzip archives"}, {"tar.xz", "lzma archives"}},
                                                                 temporary_folders{},
                                                                 logger(spdlog::get("jpod"))
    {
    }
    fs::path build_system_resolver::local_folder()
    {
        return fs::path(local_directory);
    }
    fs::path build_system_resolver::stage_path(const std::string &label, std::error_code &error)
    {
        // need to re-work this call, it's goofy
        std::string identifier;
        if(auto position = stage_names.find(label); position == stage_names.end())
        {
            // fail with error
        } else {
            identifier = position->second;
        }

        if(temporary_folders.find(identifier) == temporary_folders.end())
        {
                return create_temporary_folder(identifier, error);
        }
        return temporary_folders[identifier];
    }
    fs::path build_system_resolver::destination_path(const std::string &identifier, std::error_code &error)
    {
        if(temporary_folders.find(identifier) == temporary_folders.end())
        {
            return create_temporary_folder(identifier, error);
        }
        return temporary_folders[identifier];
    }
    fs::path build_system_resolver::image_path()
    {
        return image_folder;
    }
    fs::path build_system_resolver::generate_image_path(const std::string &identifier, std::error_code &error)
    {
        // generate a folder in a pre-fixed path that has the ${identifier} as the final folder
        fs::path image_fs_archive = image_folder / fs::path(identifier) / fs::path("fs.tar.gz");
        if (!fs::create_directories(image_fs_archive.parent_path(), error))
        {
            logger->error("FS GEN ERR : {}", error.message());
        }
        return image_fs_archive;
    }
    void build_system_resolver::extract_image(const std::string &identifier, const std::string &image_identifier, extraction_callback callback)
    {
        // An Image is Composed of Layers, so extraction would imply opening up the layers into a target directory

        fs::path output_folder = temporary_folders[identifier];
        for (const auto &entry : fs::directory_iterator(image_folder / fs::path(image_identifier)))
        {
            if (fs::is_regular_file(entry) && extensions.find(entry.path().extension()) != extensions.end())
            {
                // perform extraction
                progress_frame frame{};
                std::error_code error{};
                auto out = initialize_writer();
                if (auto in = initialize_reader(entry, error); error)
                {
                    callback(error, frame);
                }
                else
                {
                    archive_entry *entry;
                    while (archive_read_next_header(in.get(), &entry) == ARCHIVE_OK)
                    {
                        const char *entry_name = archive_entry_pathname(entry);
                        fs::path full_path = output_folder / fs::path(std::string(entry_name));
                        archive_entry_set_pathname(entry, full_path.generic_string().c_str());
                        if (auto ec = archive_write_header(out.get(), entry); ec != ARCHIVE_OK)
                        {
                            logger->error("{}", archive_error_string(out.get()));
                        }
                        else if (archive_entry_size(entry) > 0)
                        {
                            if (error = copy_entry(in.get(), out.get()); error)
                            {
                                callback(error, frame);
                            }
                        }
                    }
                }
            }
        }
    }

    std::error_code build_system_resolver::copy_entry(struct archive *in, struct archive *out)
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
                return make_compression_error_code(ec);
            }
        }
        if (ec = archive_write_finish_entry(out); ec != ARCHIVE_OK)
        {
            logger->error("{}", archive_error_string(out));
            return make_compression_error_code(ec);
        }
        return {};
    }

    fs::path build_system_resolver::create_temporary_folder(const std::string& identifier, std::error_code& error)
    {
        fs::path output_folder = fs::temp_directory_path() / identifier;
        fs::create_directories(output_folder, error);
        if(!error)
        {
            temporary_folders.emplace(identifier, output_folder);
        }
        return output_folder;
    }

    archive_ptr build_system_resolver::initialize_reader(const fs::path &image_fs_archive, std::error_code &error)
    {
        archive_ptr arch = {
            archive_read_new(),
            [](archive *instance) -> void
            {
                archive_read_close(instance);
                archive_read_free(instance);
            }};
        archive_read_support_filter_all(arch.get());
        archive_read_support_format_raw(arch.get());
        if (auto ec = archive_read_open_filename(arch.get(), image_fs_archive.c_str(), BUFFER_SIZE); ec != ARCHIVE_OK)
        {
            logger->error("{}", archive_error_string(arch.get()));
            error = make_compression_error_code(ec);
            return {};
        }
        return arch;
    }
    archive_ptr build_system_resolver::initialize_writer()
    {
        archive_ptr arch = {
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
        archive_write_disk_set_options(arch.get(), flags);
        archive_write_disk_set_standard_lookup(arch.get());
        return arch;
    }
    build_system_resolver::~build_system_resolver()
    {
    }
}