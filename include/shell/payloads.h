#ifndef __JPOD_SERVICE_SHELL_PAYLOADS__
#define __JPOD_SERVICE_SHELL_PAYLOADS__

#include <msgpack/msgpack.hpp>

namespace shell
{
  struct initiation_order
  {
    std::string container;
    std::string environment;
    uint32_t columns;
    uint32_t rows;

    template <class T>
    void pack(T &pack)
    {
      pack(container, environment, columns, rows);
    }
  };

  struct resize_order
  {
    std::string environment;
    uint32_t columns;
    uint32_t rows;
    template <class T>
    void pack(T &pack)
    {
      pack(environment, columns, rows);
    }
  };

  struct input_order
  {
    std::vector<uint8_t> content;
    template <class T>
    void pack(T &pack)
    {
      pack(content);
    }
  };

  struct output_response
  {
    std::vector<uint8_t> content;
    template <class T>
    void pack(T &pack)
    {
      pack(content);
    }
  };

  struct session_message
  {
    std::string type;
    std::string data;
    template <class T>
    void pack(T &pack)
    {
      pack(type, data);
    }
  };

} // namespace shell
#endif // __JPOD_SERVICE_SHELL_PAYLOADS__