#include <domain/images/http/asio_client.h>
#include <domain/images/http/secure_connection.h>
#include <domain/images/http/insecure_connection.h>
#include <asio/io_context.hpp>
#include <sole.hpp>

namespace domain::images::http
{
    asio_client::asio_client(asio::io_context &context, uint32_t pool_size) : context(context),
                                                                              access_mutex()
    {

        for (auto i = 0; i < pool_size; ++i)
        {
            auto identifier = sole::uuid4().str();
            available_insecured_connections.try_emplace(identifier, std::make_shared<insecure_connection>(context, identifier, *this));
        }
    }
    void asio_client::download(const std::string &path,const std::map<std::string, std::string> &headers, std::shared_ptr<download_destination> sink, report_callback callback)
    {
        std::error_code error;
        if (auto uri = internal::parse_url(path, error); error)
        {
            callback(error, {});
        }
        else if (access_mutex.try_lock())
        {
            if (auto available = available_insecured_connections.begin(); available != available_insecured_connections.end())
            {
                std::string identifier(available->first);
                auto entry = available_insecured_connections.extract(available);
                busy_insecure_connections.insert(std::move(entry));
                busy_insecure_connections.at(identifier)->download(uri, headers, std::move(sink), std::move(callback));
            }
            access_mutex.unlock();
        }
        else
        {
            callback(std::make_error_code(std::errc::resource_unavailable_try_again), {});
        }
    }
    void asio_client::execute(const request &req, response_callback callback)
    {
        if (access_mutex.try_lock())
        {
            if (auto available = available_insecured_connections.begin(); available != available_insecured_connections.end())
            {
                std::string identifier(available->first);
                auto entry = available_insecured_connections.extract(available);
                busy_insecure_connections.insert(std::move(entry));
                busy_insecure_connections.at(identifier)->execute(req, std::move(callback));
            }
            access_mutex.unlock();
        }
        else
        {
            callback(std::make_error_code(std::errc::resource_unavailable_try_again), {});
        }
    }

    void asio_client::upload(
        const std::string &path,
        const std::map<std::string, std::string> &headers,
        const fs::path &file_path,
        upload_callback callback)
    {
        std::error_code error;
        if (auto uri = internal::parse_url(path, error); error)
        {
            callback(error, {});
        }
        else if (access_mutex.try_lock())
        {
            if (auto available = available_insecured_connections.begin(); available != available_insecured_connections.end())
            {
                std::string identifier(available->first);
                auto entry = available_insecured_connections.extract(available);
                busy_insecure_connections.insert(std::move(entry));
                busy_insecure_connections.at(identifier)->upload(uri, headers, file_path, std::move(callback));
            }
            access_mutex.unlock();
        }
        else
        {
            callback(std::make_error_code(std::errc::resource_unavailable_try_again), {});
        }
    }
    void asio_client::available(const std::string &id)
    {
        if (access_mutex.try_lock())
        {
            if (auto busy = busy_insecure_connections.find(id); busy != busy_insecure_connections.end())
            {
                std::string identifier(busy->first);
                auto entry = busy_insecure_connections.extract(busy);
                available_insecured_connections.insert(std::move(entry));
            }
            access_mutex.unlock();
        }
    }
    asio_client::~asio_client()
    {
    }
}