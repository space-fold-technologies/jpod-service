
#include <domain/images/http/request_builder.h>
#include <domain/images/http/request.h>
#include <domain/images/http/uri.h>

namespace domain::images::http
{
    request_builder &request_builder::post()
    {
        this->method = "POST";
        return *this;
    }

    request_builder &request_builder::put()
    {
        this->method = "PUT";
        return *this;
    }

    request_builder &request_builder::remove()
    {
        this->method = "DELETE";
        return *this;
    }

    request_builder &request_builder::get()
    {
        this->method = "GET";
        return *this;
    }

    request_builder &request_builder::address(std::string url)
    {
        this->url = std::move(url);
        return *this;
    }
    request_builder &request_builder::body(std::vector<uint8_t> content)
    {
        this->content = std::move(content);
        return *this;
    }

    request_builder &request_builder::add_header(std::string key, std::string value)
    {
        this->headers.emplace(std::move(key), std::move(value));
        return *this;
    }

    request request_builder::build(std::error_code &error)
    {
        auto uri = internal::parse_url(this->url, error);
        return request(std::move(method), std::move(uri), std::move(content), std::move(content_type), std::move(headers));
    }
}