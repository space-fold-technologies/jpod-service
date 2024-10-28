#include <core/oci/upload_task.h>
#include <core/oci/upload_task_listener.h>
#include <core/http/file_transfer_payloads.h>
#include <core/http/file_transfer_client.h>
#include <core/http/async_client.h>
#include <spdlog/spdlog.h>
#include <asio/post.hpp>

namespace core::oci
{
    upload_task::upload_task(asio::io_context &context, upload_details details, upload_task_listener &listener) : context(context),
                                                                                                                  details(std::move(details)),
                                                                                                                  listener(listener),
                                                                                                                  upload_started(false),
                                                                                                                  logger(spdlog::get("jpod"))
    {
        client = std::make_unique<core::http::file_transfer_client>(context, this->details.provider);
    }
    void upload_task::set_location(std::string location)
    {
        this->location = location;
    }
    void upload_task::start()
    {
        if (location.empty())
        {
            logger->warn("no location set for upload");
        }
        else
        {
            if (client)
            {
                asio::post(
                    context,
                    [self = shared_from_this()]()
                    {
                        core::http::upload_request request{};
                        request.url = self->location;
                        request.method = "PATCH";
                        request.headers.try_emplace("Authorization", fmt::format("Bearer {}", self->details.token));
                        request.file_name = self->details.file_path.filename();
                        request.file_path = self->details.file_path;
                        self->client->upload(
                            request,
                            std::bind(
                                &upload_task::on_status_update,
                                self->shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2));
                    });
            }
        }
    }
    void upload_task::on_status_update(const std::error_code &error, const core::http::upload_status &status)
    {
        if (error)
        {
            listener.on_upload_failure(details.image_identifier, details.digest, error);
        }
        else
        {
            if (!upload_started)
            {
                upload_started = true;
                listener.on_upload_started(details.image_identifier, details.digest);
            }
            if (!status.complete)
            {
                listener.on_upload_update(details.image_identifier, details.digest, status.current, status.total);
            }
            else
            {
                confirm_upload();
            }
        }
    }
    void upload_task::abort()
    {
    }
    void upload_task::confirm_upload()
    {
        auto client = std::make_unique<core::http::async_client>(this->details.provider);
        http::request_details request{};
        request.headers.emplace("Content-Length", "0");
        request.headers.emplace("Authorization", fmt::format("Bearer {}", details.token));
        request.path = fmt::format("{}?digest={}", location, details.digest);
        auto response_callback = [self = shared_from_this()](const std::error_code &error, const http::response &response)
        {
            if (error)
            {
                self->listener.on_upload_failure(
                    self->details.image_identifier,
                    self->details.digest,
                    error);
            }
            else
            {
                self->listener.on_upload_complete(
                    self->details.image_identifier,
                    self->details.digest,
                    self->location);
            }
        };
        client->put(request, response_callback);
    }
    upload_task::~upload_task()
    {
    }
}
