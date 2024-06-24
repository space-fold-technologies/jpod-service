#ifndef __DAEMON_CORE_HTTP_REQUEST__
#define __DAEMON_CORE_HTTP_REQUEST__

#include <core/http/uri.h>
#include <tl/expected.hpp>
#include <fmt/format.h>
#include <system_error>
#include <vector>
#include <string>

using namespace tl;

namespace core::http
{
    struct request
    {
        std::vector<uint8_t> content;
        internal::uri url;
    };

    auto compose_request(
        const std::string &path,
        const http_headers &headers,
        std::string method) -> expected<request, std::error_code>
    {
        std::error_code error{};
        request r{};
        if (r.url = internal::parse_url(path, error); error)
        {
            return tl::make_unexpected(error);
        }
        auto header_line = fmt::format("{} {} HTTP/1.1\r\n"
                                       "Host: {}\r\n"
                                       "User-Agent: jpod client\r\n",
                                       method,
                                       r.url.full_path(),
                                       r.url.full_host());
        std::for_each(headers.begin(), headers.end(), [&header_line](const auto &entry)
                      { header_line += fmt::format("{}: {}\r\n", entry.first, entry.second); });
        header_line += "\r\n";
        r.content.reserve(header_line.size());
        r.content.assign(header_line.begin(), header_line.end());
        return r;
    }

    auto compose_request(
        const request_details &details,
        std::string method) -> expected<request, std::error_code>
    {
        std::error_code error{};
        request r{};
        if (r.url = internal::parse_url(details.path, error); error)
        {
            return tl::make_unexpected(error);
        }
        else
        {
            auto header_line = fmt::format("{} {} HTTP/1.1\r\n"
                                           "Host: {}\r\n"
                                           "User-Agent: jpod client\r\n"
                                           "Content-Type: {}\r\n"
                                           "Content-Length: {}\r\n",
                                           method,
                                           r.url.path,
                                           r.url.full_host(),
                                           details.content_type,
                                           details.content.size());
            auto headers = details.headers;
            std::for_each(headers.begin(), headers.end(), [&header_line](const auto &entry)
                          { header_line += fmt::format("{}: {}\r\n", entry.first, entry.second); });
            header_line += "\r\n";
            r.content.reserve(header_line.size() + details.content.size());
            r.content.assign(header_line.begin(), header_line.end());
            if (!details.content.empty())
            {
                r.content.insert(r.content.end(), details.content.begin(), details.content.end());
            }
            return r;
        }
    }
}

#endif // __DAEMON_CORE_HTTP_REQUEST__