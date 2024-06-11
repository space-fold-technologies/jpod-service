#include <core/oci/layer_download_task.h>
#include <core/oci/download_task_listener.h>
#include <core/http/file_transfer_client.h>
#include <core/http/file_transfer_payloads.h>
#include <asio/post.hpp>
#include <spdlog/spdlog.h>

namespace core::oci
{
    layer_download_task::layer_download_task(
        asio::io_context &context,
        download_details details,
        download_task_listener &listener,
        std::size_t buffer_size) : context(context),
                                   details(std::move(details)),
                                   buffer_size(buffer_size),
                                   listener(listener),
                                   download_started(false),
                                   logger(spdlog::get("jpod"))
    {
        // use this to
        auto extension = this->details.media_type.find_last_of(".tar+gzip") != std::string::npos ? "tar.gz" : "tar";
        file_path = fs::path(this->details.folder / fs::path(fmt::format("layer_{0}.{1}", this->details.index, extension)))
                        .generic_string();
        client = std::make_unique<core::http::file_transfer_client>(context, this->details.provider);
        logger->info("INDEX: {} URL: {}", this->details.index, this->details.layer_url);
        logger->info("FILE PATH :{}", file_path);
    }
    bool layer_download_task::is_valid()
    {
        if (!fs::exists(file_path))
        {
            auto path = fs::path(file_path);
            fs::create_directories(path.parent_path());
            fs::permissions(path.parent_path(),
                            std::filesystem::perms::owner_all | std::filesystem::perms::group_all,
                            std::filesystem::perm_options::add);
            return true;
        }
        if (!fs::is_regular_file(file_path))
        {
            return false;
        }
        auto permissions = fs::status(file_path).permissions();
        return fs::perms::none != (fs::perms::owner_write & permissions) || fs::perms::none != (fs::perms::group_write & permissions);
    }
    std::size_t layer_download_task::chunk_size() const
    {
        return buffer_size;
    }
    std::size_t layer_download_task::write(const std::vector<uint8_t> &data)
    {
        logger->info("writing {} bytes to file", data.size());
        std::ofstream file(file_path, std::ios::binary | std::ios::app);
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
        return data.size();
    }
    void layer_download_task::update_target(std::string path)
    {
        details.layer_url = path;
    }
    void layer_download_task::start()
    {
        if (client)
        {
            asio::post(
                context,
                [self = shared_from_this()]()
                {
                    core::http::download_request request{};
                    auto extension = self->details.media_type.find_last_of(".tar+gzip") != std::string::npos ? "tar.gz" : "tar";
                    request.name = fmt::format("layer_{0}.{1}", self->details.index, extension);
                    request.headers.try_emplace("Authorization", fmt::format("Bearer {}", self->details.token));
                    request.sink = self->shared_from_this();
                    request.url = self->details.layer_url;
                    request.size = self->details.layer_size;
                    self->client->download(request, std::bind(&layer_download_task::on_download_update, self->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
                });
        }
    }
    void layer_download_task::abort()
    {
        // stop the client and remove the file
        logger->warn("removing layer: {}", file_path);
        std::error_code error{};
        if(fs::remove(fs::path(file_path), error); error) 
        {
            listener.on_download_failure(details.image_digest, details.layer_digest, error);
        }
    }
    void layer_download_task::on_download_update(const std::error_code &error, const core::http::download_status &status)
    {
        if (error)
        {
            listener.on_download_failure(details.image_digest, details.layer_digest, error);
        }
        else
        {
            if (!download_started)
            {
                download_started = true;
                listener.on_download_started(details.image_digest, details.layer_digest);
            }
            update_details update{};
            update.image_digest = details.image_digest;
            update.layer_digest = details.layer_digest;
            update.current = status.current;
            update.total = status.total;
            if (!status.complete)
            {
                listener.on_download_update(update);
            }
            else
            {
                listener.on_download_complete(details.image_digest, details.layer_digest);
            }
        }
    }
    layer_download_task::~layer_download_task()
    {
    }
}