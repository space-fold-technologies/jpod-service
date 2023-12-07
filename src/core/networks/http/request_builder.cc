

#include <core/networks/http/request_builder.h>
#include <core/networks/http/request.h>

namespace core::networks::http
{
    RequestBuilder &RequestBuilder::post()
    {
        this->method = "POST";
        return *this;
    }

    RequestBuilder &RequestBuilder::put()
    {
        this->method = "PUT";
        return *this;
    }

    RequestBuilder &RequestBuilder::remove()
    {
        this->method = "DELETE";
        return *this;
    }

    RequestBuilder &RequestBuilder::get()
    {
        this->method = "GET";
        return *this;
    }

    RequestBuilder &RequestBuilder::address(std::string url)
    {
        this->url = std::move(url);
        return *this;
    }
    RequestBuilder &RequestBuilder::body(std::vector<uint8_t> content)
    {
        this->content = std::move(content);
        return *this;
    }

    RequestBuilder &RequestBuilder::add_header(std::string key, std::string value)
    {
        this->headers.emplace(std::move(key), std::move(value));
        return *this;
    }

    tl::expected<Request, std::string> RequestBuilder::build()
    {
        auto result = http::parse_url(this->url);
        if (!result.has_value())
        {
            return tl::make_unexpected("invalid url");
        }
        if (method == "GET" && !content.empty() || method == "DELETE" && !content.empty())
        {
            return tl::make_unexpected("GET or DELETE cannot have bodies");
        }
        Request request(std::move(method), std::move(*result), std::move(content), std::move(content_type), std::move(headers));
        return {request};
    }
}