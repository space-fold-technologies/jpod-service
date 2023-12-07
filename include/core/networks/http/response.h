#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_RESPONSE__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_RESPONSE__

#include <string>
#include <vector>
#include <map>
#include <system_error>
#include <sstream>
#include <asio/buffers_iterator.hpp>
#include <tl/expected.hpp>

namespace core::networks::http
{
    struct hash
    {
    public:
        std::size_t operator()(const std::string &str) const noexcept
        {
            std::size_t h = 0;
            std::hash<int> hash;
            for (auto c : str)
                h ^= hash(tolower(c)) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
    struct equal
    {
    public:
        bool operator()(const std::string &str1, const std::string &str2) const noexcept
        {
            return str1.size() == str2.size() &&
                   std::equal(str1.begin(), str1.end(), str2.begin(), [](char a, char b)
                              { return tolower(a) == tolower(b); });
        }
    };

    struct Response
    {
    public:
        inline auto has_body() const -> bool
        {
            return headers.find("Content-Length") != headers.end() && headers.find("Accept-Ranges") == headers.end();
        };
        inline auto is_chunked() const -> bool
        {
            if (auto pos = headers.find("Transfer-Encoding"); pos != headers.end() && pos->second == "chunked")
                return true;
            return false;
        };
        inline auto is_event_stream() const -> bool
        {
            if (auto pos = headers.find("Content-Type"); pos != headers.end() && pos->second == "text/event-stream")
                return true;
            return false;
        };
        inline auto is_closed() const -> bool
        {
            if (auto pos = headers.find("Connection"); pos != headers.end() && pos->second == "close" || version < "1.1")
                return true;
            return false;
        };
        inline auto content_length() const -> std::size_t
        {
            if (auto pos = headers.find("Content-Length"); pos != headers.end())
            {
                return std::stoul(pos->second);
            }
            return 0;
        };
        inline auto content_type() const -> std::string
        {
            if (auto pos = headers.find("Content-Type"); pos != headers.end())
            {
                return pos->second;
            }
            return "";
        };
        inline auto accepts_ranges() const -> bool
        {
            if (headers.find("Content-Length") != headers.end())
            {
                if (auto pos = headers.find("Accept-Ranges"); pos != headers.end() && pos->second != "none")
                {
                    return true;
                }
                return false;
            }
            return false;
        }
        inline auto unit() const -> std::string
        {
            if (auto pos = headers.find("Accept-Ranges"); pos != headers.end() && pos->second != "none")
            {
                return pos->second;
            }
            return "";
        }
        std::unordered_multimap<std::string, std::string, hash, equal> headers;
        std::vector<uint8_t> data;
        std::string status_code;
        std::string version;
        std::error_code err;
    };

    inline auto error(std::error_code err) -> Response
    {
        Response response{};
        response.err = err;
        return response;
    };

    inline auto parse_headers(std::stringstream &stream) -> std::unordered_multimap<std::string, std::string, hash, equal>
    {
        std::unordered_multimap<std::string, std::string, hash, equal> result;
        std::string line;
        std::size_t param_end;
        while (getline(stream, line) && (param_end = line.find(':')) != std::string::npos)
        {
            std::size_t value_start = param_end + 1;
            while (value_start + 1 < line.size() && line[value_start] == ' ')
                ++value_start;
            if (value_start < line.size())
                result.emplace(line.substr(0, param_end), line.substr(value_start, line.size() - value_start - (line.back() == '\r' ? 1 : 0)));
        }
        return result;
    };

    inline auto parse_response(std::string &content) -> tl::expected<Response, std::string>
    {
        Response response{};
        std::string line;
        std::size_t version_end;
        std::stringstream stream(content);
        if (getline(stream, line) && (version_end = line.find(' ')) != std::string::npos)
        {
            if (5 < line.size())
                response.version = line.substr(5, version_end - 5);
            else
                return tl::make_unexpected("incorrectly formatted response");
            if ((version_end + 1) < line.size())
                response.status_code = line.substr(version_end + 1, line.size() - (version_end + 1) - (line.back() == '\r' ? 1 : 0));
            else
                return tl::make_unexpected("incorrectly formatted response");
            response.headers = parse_headers(stream);
            return {response};
        };
        return tl::make_unexpected("no lines present");
    };

}
#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_RESPONSE__