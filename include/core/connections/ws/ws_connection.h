#ifndef __CORE_CONNECTIONS_WEBSOCKET_CONNECTION__
#define __CORE_CONNECTIONS_WEBSOCKET_CONNECTION__

#include <core/connections/connection.h>
#include <asio/ip/tcp.hpp>
#include <asio/strand.hpp>
#include <asio/streambuf.hpp>
#include <asio/steady_timer.hpp>
#include <functional>
#include <map>
#include <memory>
#include <array>
#include <mutex>
#include <vector>
#include <deque>
#include <atomic>
#include <core/connections/ws/frame.h>

#if (defined(ASIO_STANDALONE) && ASIO_VERSION >= 101800)
#include <asio/any_io_executor.hpp>
using strand = asio::strand<asio::any_io_executor>;
#else
#include <asio/executor.hpp>
using strand = asio::strand<asio::executor>;
#endif

namespace spdlog
{
  class logger;
}

namespace core::connections
{
  class ConnectionListener;
}

namespace core::connections::ws
{
  constexpr uint8_t OP_CONTINUE = 0x0;
  constexpr uint8_t OP_TEXT = 0x1;
  constexpr uint8_t OP_BINARY = 0x2;
  constexpr uint8_t OP_CLOSE = 0x8;
  constexpr uint8_t OP_PING = 0x9;
  constexpr uint8_t OP_PONG = 0xA;

  class Request;
  class PropertiesListener;
  class WebsocketConnection : public Connection, public std::enable_shared_from_this<WebsocketConnection>
  {
  public:
    WebsocketConnection(asio::ip::tcp::socket socket, PropertiesListener& properties_callback);
    virtual ~WebsocketConnection() = default;
    void initiate();
    void connect() override;
    void register_callback(std::shared_ptr<ConnectionListener> listener) override;
    void write(const std::vector<uint8_t> &payload) override;
    void disconnect() override;

  private:
    void read_request_handshake();
    void response_to_handshake(const Request &request);
    void read_payload_header();
    void read_payload_content(bool fin, uint8_t opcode, bool has_mask, std::size_t payload_length);
    void read_medium_payload(bool fin, uint8_t opcode, bool has_mask);
    void read_large_payload(bool fin, uint8_t opcode, bool has_mask);
    void handle_fragment(bool fin, uint8_t opcode, const std::vector<uint8_t> &content);
    void encode_length_to_frame(std::vector<uint8_t> &header, bool send_mask, const std::array<uint8_t, 4> &mask, std::size_t length);
    void send_payload(uint8_t opcode, bool fin, bool send_mask, const std::vector<uint8_t> &content);
    void send_payload(uint8_t opcode, bool fin, const std::array<uint8_t, 4> &mask, const std::vector<uint8_t> &content);
    void send_from_queue();
    void cancel_timeout();
    void initiate_timeout();
    void status_report(int status, std::string &reason, std::string &details);
    void on_ping(const std::vector<uint8_t> &content);
    void on_pong(const std::vector<uint8_t> &content);

  private:
    asio::ip::tcp::socket socket;
    PropertiesListener& properties_callback;
    std::shared_ptr<ConnectionListener> listener;
    std::unique_ptr<message> message_ptr;
    std::deque<std::vector<uint8_t>> sending_queue;
    std::shared_ptr<spdlog::logger> logger;
    asio::streambuf stream_buffer;
    std::mutex lock;
    std::unique_ptr<asio::steady_timer> timer;
  };
} // namespace core::connections::ws
#endif // __CORE_CONNECTIONS_WEBSOCKET_CONNECTION__