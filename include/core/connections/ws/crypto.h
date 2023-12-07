#ifndef __CORE_CONNECTIONS_WEBSOCKET_CRYPTO__
#define __CORE_CONNECTIONS_WEBSOCKET_CRYPTO__
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include <openssl/evp.h>

namespace core::connections::ws {
constexpr std::size_t buffer_size = 131072;

class Crypto {
public:
  static std::string base64_encode(const std::string &input) noexcept;
  static std::string base64_decode(const std::string &base64) noexcept;
  static std::string message_digest(const std::string &str, const EVP_MD *evp_md, std::size_t digest_length) noexcept;
  static std::string sha1(const std::string &input, std::size_t iterations = 1) noexcept;
};
} // namespace core::connections::ws
#endif // __CORE_CONNECTIONS_WEBSOCKET_CRYPTO__