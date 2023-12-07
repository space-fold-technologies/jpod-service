#ifndef __CORE_CONNECTIONS_WEBSOCKET_HEADER_PARSER__
#define __CORE_CONNECTIONS_WEBSOCKET_HEADER_PARSER__
#include <istream>
#include <tl/expected.hpp>
#include <unordered_map>

namespace core::connections::ws
{
  inline bool case_insensitive_equal(const std::string &str1,
                                     const std::string &str2) noexcept
  {
    return str1.size() == str2.size() &&
           std::equal(str1.begin(), str1.end(), str2.begin(),
                      [](char a, char b)
                      { return tolower(a) == tolower(b); });
  }

  class hash
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

  class equals
  {
  public:
    bool operator()(const std::string &str1,
                    const std::string &str2) const noexcept
    {
      return case_insensitive_equal(str1, str2);
    }
  };

  using header_map = std::unordered_map<std::string, std::string, hash, equals>;

  struct Request
  {
    header_map headers;
    std::string http_version;
    std::string status_code;
    std::string method;
    std::string path;

  public:
    static tl::expected<Request, std::string>
    parse(std::istream &stream) noexcept
    {
      std::string method;
      std::string http_version;
      std::string status_code;
      std::string path;
      std::string query_string;
      header_map headers = parse_headers(stream);
      if (auto err = parse_details(stream, method, http_version, path, query_string); !err.empty())
      {
        return tl::make_unexpected(err);
      }
      return Request{headers, http_version, status_code};
    }

  private:
    static std::string parse_details(std::istream &input_stream,
                                     std::string &method,
                                     std::string &version,
                                     std::string &path,
                                     std::string &query_string)
    {
      std::string line;
      std::size_t method_end;
      if (std::getline(input_stream, line) && (method_end = line.find(' ')) != std::string::npos)
      {
        method = line.substr(0, method_end);

        std::size_t query_start = std::string::npos;
        std::size_t path_and_query_string_end = std::string::npos;
        for (std::size_t i = method_end + 1; i < line.size(); ++i)
        {
          if (line[i] == '?' && (i + 1) < line.size() && query_start == std::string::npos)
            query_start = i + 1;
          else if (line[i] == ' ')
          {
            path_and_query_string_end = i;
            break;
          }
        }
        if (path_and_query_string_end != std::string::npos)
        {
          if (query_start != std::string::npos)
          {
            path = line.substr(method_end + 1, query_start - method_end - 2);
            query_string = line.substr(query_start, path_and_query_string_end - query_start);
          }
          else
            path = line.substr(method_end + 1, path_and_query_string_end - method_end - 1);

          std::size_t protocol_end;
          if ((protocol_end = line.find('/', path_and_query_string_end + 1)) != std::string::npos)
          {
            if (line.compare(path_and_query_string_end + 1, protocol_end - path_and_query_string_end - 1, "HTTP") != 0)
              return std::string("HTTP protocol not specified");
            version = line.substr(protocol_end + 1, line.size() - protocol_end - 2);
          }
        }
      }
      return "";
    }
    static header_map parse_headers(std::istream &stream) noexcept
    {
      header_map headers;
      std::string line;
      std::size_t param_end;
      while (std::getline(stream, line))
      {
        param_end = line.find_first_of(':');
        if (param_end != std::string::npos)
        {
          std::size_t value_start = param_end + 1;
          while (value_start + 1 < line.size() && line[value_start] == ' ')
            ++value_start;
          if (value_start < line.size())
            headers.emplace(line.substr(0, param_end),
                            line.substr(value_start, line.size() - value_start - (line.back() == '\r' ? 1 : 0)));
        }
      }
      return headers;
    }
  };

} // namespace ws

#endif // __REMOTE_SHELL_WS_HEADER_PARSER__