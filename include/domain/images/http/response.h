#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_RESPONSE__
#define __DAEMON_DOMAIN_IMAGES_HTTP_RESPONSE__

#include <string>
#include <vector>
#include <map>
#include <system_error>
#include <sstream>
#include <unordered_map>

namespace domain::images::http
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

    enum class http_error
    {
        ok,
        incorrectly_formatted_response,
        no_lines_present
    };

    inline std::map<http_error, std::string> http_error_map =
        {
            {http_error::ok, "ok"},
            {http_error::incorrectly_formatted_response, "incorrectly formatted response"},
            {http_error::no_lines_present, "no lines present"}};

    struct http_failure_category : public std::error_category
    {
        http_failure_category() {}
        virtual ~http_failure_category() = default;
        http_failure_category(const http_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "http response failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_parser_error_code("unknown http error");
            if (auto position = http_error_map.find(static_cast<http_error>(ec)); position != http_error_map.end())
            {
                return position->second;
            }
            return unknown_parser_error_code;
        }
    };

    inline const http_failure_category &__http_failure_category()
    {
        static http_failure_category fc;
        return fc;
    }

    inline const std::error_code make_error_code(http_error ec) noexcept
    {

        return std::error_code{static_cast<int>(ec), __http_failure_category()};
    };

    struct response
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

    inline auto parse_response(std::string &content, std::error_code &error) -> response
    {
        response response{};
        std::string line;
        std::size_t version_end;
        std::stringstream stream(content);
        if (getline(stream, line) && (version_end = line.find(' ')) != std::string::npos)
        {
            if (5 < line.size())
                response.version = line.substr(5, version_end - 5);
            else
            {
                error = make_error_code(http_error::incorrectly_formatted_response);
                return {};
            }
            if ((version_end + 1) < line.size())
                response.status_code = line.substr(version_end + 1, line.size() - (version_end + 1) - (line.back() == '\r' ? 1 : 0));
            else
            {
                error = make_error_code(http_error::incorrectly_formatted_response);
                return {};
            }
            response.headers = parse_headers(stream);
            return {response};
        };
        error = make_error_code(http_error::no_lines_present);
        return {};
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_HTTP_RESPONSE__