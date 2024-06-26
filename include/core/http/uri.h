#ifndef __DAEMON_CORE__HTTP_URI__
#define __DAEMON_CORE__HTTP_URI__

#include <string>
#include <cstdint>
#include <algorithm>
#include <map>
#include <system_error>
#include <fmt/format.h>

namespace core::http::internal
{
    enum class parser_error
    {
        ok,
        un_initialized,
        no_url_character,
        invalid_scheme_name,
        no_double_slash,
        no_at_sign,
        un_expected_end_of_line,
        no_slash
    };

    inline std::map<parser_error, std::string> parser_error_map =
        {
            {parser_error::ok, "ok"},
            {parser_error::un_initialized, "uninitialized"},
            {parser_error::no_url_character, "no url character"},
            {parser_error::invalid_scheme_name, "invalid scheme name"},
            {parser_error::no_double_slash, "no double slash"},
            {parser_error::no_at_sign, "no @ sign"},
            {parser_error::un_expected_end_of_line, "unexpected end of line"},
            {parser_error::no_slash, "no slash"}};

    struct uri_failure_category : public std::error_category
    {
        uri_failure_category() {}
        virtual ~uri_failure_category() = default;
        uri_failure_category(const uri_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "uri failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_parser_error_code("unknown uri failure");
            if (auto position = parser_error_map.find(static_cast<parser_error>(ec)); position != parser_error_map.end())
            {
                return position->second;
            }
            return unknown_parser_error_code;
        }
    };

    inline const uri_failure_category &__uri_failure_category()
    {
        static uri_failure_category fc;
        return fc;
    }

    inline const std::error_code make_error_code(parser_error ec) noexcept
    {

        return std::error_code{static_cast<int>(ec), __uri_failure_category()};
    };

    struct uri
    {
        std::string scheme;
        std::string host;
        uint16_t port;
        std::string path;
        std::string query;
        std::string original;

    public:
        std::string full_path() const
        {
            if (!query.empty())
            {
                return fmt::format("{}?{}", path, query);
            }
            return path;
        }

        std::string full_host() const
        {
            std::string _full_host = host;
            if (port != 80 && port != 443)
            {
                _full_host = fmt::format("{}:{}", _full_host, port);
            }
            return _full_host;
        }
    };

    inline std::string parse_host(const std::string &content, std::error_code &error)
    {
        if (auto pos = content.find_first_of("//"); pos != std::string::npos)
        {
            auto remainder = content.substr(pos + 2);
            if (auto domain_end_pos = remainder.find_first_of("/?#"); domain_end_pos != std::string::npos)
            {
                return remainder.substr(0, domain_end_pos);
            }
            return remainder;
        }
        else
        {
            error = make_error_code(parser_error::no_double_slash);
        }
        return "";
    }

    inline std::string parse_path(const std::string &content)
    {
        if (auto pos = content.find_first_of("/"); pos != std::string::npos)
        {
            if (auto end_pos = content.find_first_of("?"); end_pos != std::string::npos)
            {
                return content.substr(pos, end_pos - pos);
            }
            return content.substr(pos);
        }
        return content;
    }

    inline uint16_t parse_port(bool secured, bool &found_port, const std::string &content)
    {
        uint16_t result = 0;
        found_port = true;
        if (content.find_first_of(":") != std::string::npos)
        {
            if (content.find_first_of("/") != std::string::npos)
            {
                result = static_cast<uint16_t>(std::atoi(content.substr(content.find_first_of(":") + 1, content.find_first_of("/") - 1).c_str()));
            }
            else if (content.find_first_of("?") != std::string::npos)
            {
                result = static_cast<uint16_t>(std::atoi(content.substr(content.find_first_of(":") + 1, content.find_first_of("?") - 1).c_str()));
            }
        }
        if (result == 0 && secured)
        {
            found_port = false;
            result = 443;
        }
        else
        {
            found_port = false;
            result = 80;
        }

        return result;
    }

    inline uri parse_url(const std::string &url, std::error_code &error)
    {
        uri uri{};
        uri.original = std::string(url);
        if (auto pos = url.find_first_of(":"); pos == url.npos)
        {
            error = make_error_code(parser_error::invalid_scheme_name);
        }
        else
        {

            uri.scheme = url.substr(0, pos);
            std::transform(uri.scheme.begin(), uri.scheme.end(), uri.scheme.begin(), ::tolower);
            auto remainder = url.substr(url.find_first_of(":") + 1);
            if (uri.host = parse_host(remainder, error); error)
            {
                return {};
            }
            else
            {
                remainder.erase(remainder.find("//"), 2);
                remainder.erase(remainder.find(uri.host), uri.host.length());
            }
            bool has_port;
            uri.port = parse_port(uri.scheme.find("https") != std::string::npos, has_port, remainder);
            if (has_port)
            {
                auto port = fmt::format(":{}", uri.port);
                if (auto port_pos = remainder.find_first_of(port); port_pos != std::string::npos)
                {
                    remainder.erase(port_pos, port.length());
                }
            }
            uri.path = parse_path(remainder);
            uri.query = remainder.find_first_of("?") != std::string::npos ? remainder.substr(remainder.find_first_of("?") + 1) : "";
        }
        return uri;
    }
}
#endif // __DAEMON_CORE__HTTP_URI__