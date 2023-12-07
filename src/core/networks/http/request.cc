
#include <core/networks/http/request.h>
#include <core/networks/http/request_builder.h>
#include <fmt/format.h>
#include <algorithm>

namespace core::networks::http
{
    Request::Request(
        std::string method,
        URI uri,
        std::vector<uint8_t> data,
        std::string content_type,
        std::map<std::string, std::string> headers) : method(std::move(method)),
                                                      uri(std::move(uri)),
                                                      data(std::move(data)),
                                                      content_type(std::move(content_type)),
                                                      headers(std::move(headers))

    {
    }

    void Request::write(asio::streambuf &buffer)
    {
        std::ostream stream(&buffer);
        stream << header(method, uri, content_type);
        std::string content(data.begin(), data.end());
        stream.write(content.data(), content.size());
    }

    std::string Request::header(const std::string &method, const URI &uri, const std::string &content_type)
    {
        auto header_line = fmt::format("{} {} HTTP/1.1\r\n"
                                       "Host: {}\r\n"
                                       "User-Agent: jpod::client\r\n"
                                       "Content-Type: {}\r\n"
                                       "Content-Length: {}\r\n",
                                       method,
                                       uri.path,
                                       fmt::format("{}:{}", uri.host, uri.port),
                                       content_type,
                                       data.size());

        std::for_each(headers.begin(), headers.end(), [&header_line](const auto &entry)
                      { header_line += fmt::format("{}: {}\r\n", entry.first, entry.second); });
        return header_line + "\r\n";
    }
    RequestBuilder Request::builder()
    {
        return RequestBuilder{};
    }
}