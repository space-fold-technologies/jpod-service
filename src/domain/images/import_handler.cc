
#include <domain/images/import_handler.h>
#include <domain/images/repository.h>
#include <domain/images/payload.h>
#include <domain/images/instructions/extraction_instruction.h>
#include <domain/images/instructions/import_instruction.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <sole.hpp>

namespace domain::images
{
    import_handler::import_handler(core::connections::connection &connection,
                                   fs::path &image_folder,
                                   std::shared_ptr<image_repository> repository) : command_handler(connection),
                                                                                   image_folder(image_folder),
                                                                                   repository(std::move(repository)),
                                                                                   logger(spdlog::get("jpod"))
    {
    }
    void import_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_import_order(payload);
        identifier = sole::uuid4().str();
        local_file_path = fs::path(order.archive_path);
        tasks.push_back(std::make_shared<extraction_instruction>(identifier, *this, *this));
        tasks.push_back(std::make_shared<import_instruction>(identifier, *repository, *this, *this));
        run_steps();
    }
    void import_handler::on_connection_closed(const std::error_code &error)
    {
        logger->info("connection closed");
    }
    void import_handler::on_instruction_initialized(std::string id, std::string name)
    {
        logger->info("id:{} initialized :{}", id, name);
    }
    void import_handler::on_instruction_data_received(std::string id, const std::vector<uint8_t> &content)
    {
        send_progress("image", content);
    }
    void import_handler::on_instruction_complete(std::string id, std::error_code err)
    {
        if (err)
        {
            send_error(err);
        }
        else
        {
            tasks.pop_front();
            run_steps();
        }
    }
    void import_handler::run_steps()
    {
        if (!tasks.empty())
        {
            auto task = tasks.front();
            task->execute();
        }
        else
        {
            logger->info("finished importing image");
            send_success(fmt::format("image import complete with ID: {}", identifier));
        }
    }
    fs::path import_handler::archive_file_path()
    {
        return local_file_path;
    }
    fs::path import_handler::image_file_path(const std::string &identifier, std::error_code &error)
    {
        fs::path image_fs_archive = image_folder / fs::path(identifier) / fs::path("fs.tar.gz");
        if (!fs::is_directory(image_fs_archive.parent_path(), error))
        {
            logger->error("fs gen err : {}", error.message());
        }
        return image_fs_archive;
    }
    fs::path import_handler::generate_image_path(const std::string &identifier, std::error_code &error)
    {
        // generate a folder in a pre-fixed path that has the ${identifier} as the final folder
        fs::path image_fs_archive = image_folder / fs::path(identifier) / fs::path("fs.tar.gz");
        if (!fs::create_directories(image_fs_archive.parent_path(), error))
        {
            logger->error("fs gen err, failed to create path: {} err : {}", image_fs_archive.generic_string(), error.message());
        }
        return image_fs_archive;
    }
    import_handler::~import_handler()
    {
        tasks.clear();
    }
}