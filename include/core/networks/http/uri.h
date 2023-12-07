#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_URI__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_URI__
#include <string>
#include <cstdint>
#include <tl/expected.hpp>
#include <algorithm>
#include <fmt/format.h>
namespace core::networks::http
{
    enum class ParserError
    {
        OK,
        UN_INITIALIZED,
        NO_URL_CHARACTER,
        INVALID_SCHEME_NAME,
        NO_DOUBLE_SLASH,
        NO_AT_SIGN,
        UN_EXPECTED_END_OF_LINE,
        NO_SLASH
    };
    struct URI
    {
        std::string scheme;
        std::string host;
        uint16_t port;
        std::string path;
        std::string query;
    };

    inline tl::expected<std::string, ParserError> parse_host(const std::string &content)
    {
        if (content.find_first_of("//") == std::string::npos)
        {
            return tl::make_unexpected(ParserError::NO_DOUBLE_SLASH);
        }
        auto remainder = content.substr(content.find_first_of("//") + 2);
        if (remainder.find_first_of(":") != std::string::npos)
        {
            return remainder.substr(0, remainder.find_first_of(":"));
        }
        else if (auto pos = remainder.find_first_of("/"); pos != std::string::npos)
        {
            return remainder.substr(0, remainder.find_first_of("/"));
        }
        else if (auto post = remainder.find_first_of("?"); pos != std::string::npos)
        {
            return remainder.substr(0, remainder.find_first_of("?"));
        }
        return remainder;
    }

    inline std::string parse_path(const std::string &content)
    {
        auto remainder = content.find_first_of("//") != std::string::npos ? content.substr(content.find_first_of("//") + 2) : content;
        if (auto pos = remainder.find_first_of("/"); pos != std::string::npos)
        {
            if (auto end_pos = remainder.find_first_of("?"); end_pos != std::string::npos)
            {
                return remainder.substr(pos, end_pos - pos);
            }
            return remainder.substr(pos);
        }
        else if (auto pos = remainder.find_first_of("?"); pos != std::string::npos)
        {
            return remainder.substr(0, pos);
        }
        return remainder;
    }

    inline uint16_t parse_port(bool secured, const std::string &content)
    {
        if (content.find_first_of(":") != std::string::npos)
        {
            if (content.find_first_of("/") != std::string::npos)
            {
                return static_cast<uint16_t>(std::atoi(content.substr(content.find_first_of(":") + 1, content.find_first_of("/") - 1).c_str()));
            }
            else if (content.find_first_of("?") != std::string::npos)
            {
                return static_cast<uint16_t>(std::atoi(content.substr(content.find_first_of(":") + 1, content.find_first_of("?") - 1).c_str()));
            }
        }
        else if (secured)
        {
            return 443;
        }
        return 80;
    }

    inline tl::expected<URI, ParserError> parse_url(const std::string &url)
    {
        URI uri{};
        if (auto pos = url.find_first_of(":"); pos != url.npos)
        {
            uri.scheme = url.substr(0, pos);
            std::transform(uri.scheme.begin(), uri.scheme.end(), uri.scheme.begin(), ::tolower);
        }
        else
        {
            return tl::make_unexpected(ParserError::INVALID_SCHEME_NAME);
        }
        auto remainder = url.substr(url.find_first_of(":") + 1);
        if (auto result = parse_host(remainder); result.has_value())
        {
            uri.host = result.value();
            remainder.erase(remainder.find(uri.host), uri.host.size());
        }
        uri.port = parse_port(uri.scheme.find("https") != std::string::npos, remainder);
        auto port = fmt::format(":{}", uri.port);
        if (remainder.find_first_of(port) != std::string::npos)
        {
            remainder.erase(remainder.find_first_of(port), port.size());
        }
        uri.path = parse_path(remainder);
        uri.query = remainder.find_first_of("?") != std::string::npos ? remainder.substr(remainder.find_first_of("?") + 1) : "";
        return uri;
    }
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_URI__