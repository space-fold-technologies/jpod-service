
#include <domain/images/http/request.h>
#include <domain/images/http/request_builder.h>
#include <algorithm>
#include <fmt/format.h>


namespace domain::images::http
{
    request::request(
        std::string method,
        internal::uri path,
        std::vector<uint8_t> data,
        std::string content_type,
        std::map<std::string, std::string> headers) : method(std::move(method)),
                                                      path(std::move(path)),
                                                      data(std::move(data)),
                                                      content_type(std::move(content_type)),
                                                      headers(std::move(headers))

    {
    }

    void request::write(asio::streambuf &buffer)
    {
        std::ostream stream(&buffer);
        stream << header(method, path, content_type);
        std::string content(data.begin(), data.end());
        stream.write(content.data(), content.size());
    }

    std::string request::header(const std::string &method, const internal::uri &uri, const std::string &content_type)
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
    std::string request::host() const
    {
        return path.host;
    }
    uint16_t request::port() const
    {
        return path.port;
    }
    request_builder request::builder()
    {
        return request_builder{};
    }
}