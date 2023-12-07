#include <shell/session.h>
#include <core/connections/connection.h>
#include <core/connections/connection_listener.h>
#include <sys/cdefs.h>
//__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/types.h>
// #include <sys/jail.h>
#include <arpa/inet.h>
// #include <jail.h>
#include <unistd.h>
#include <spdlog/spdlog.h>
#include <shell/payloads.h>

namespace shell
{
  Session::Session(std::string id, asio::io_context &context, std::shared_ptr<core::connections::Connection> connection)
      : id(std::move(id)),
        connection(std::move(connection)),
        terminal(context, *this),
        logger(spdlog::get("jpod"))
  {
    connection->register_callback(std::move(std::shared_ptr<core::connections::ConnectionListener>(this)));
    connection->connect();
  }

  void Session::on_open()
  {
    // We would need to initiate the terminal here and start initializing anything
    logger->info("session with ID: {} opened", id);
  }
  void Session::on_close()
  {
    logger->info("session with ID: {} closed", id);
  }
  void Session::on_message(const std::vector<uint8_t> &payload)
  {
    std::vector<uint8_t> content(payload.begin() + 1, payload.end());
    uint8_t type_byte = payload.at(0);
    auto type = static_cast<MessageType>(type_byte & 0x0F);

    if (type == MessageType::TERMINAL_INPUT)
    {
      terminal.write(msgpack::unpack<input_order>(content).content);
    }
    else if (type == MessageType::RESIZE_REQUEST)
    {
      auto order = msgpack::unpack<resize_order>(content);
      terminal.resize(order.columns, order.rows);
    }
    else if (type == MessageType::SESSION_REQUEST)
    {
      auto order = msgpack::unpack<initiation_order>(content);
      logger->info("starting terminal");
      std::thread t(&Session::start_session, this, order.container);
      t.join();
    }
  }

  void Session::on_error(const std::error_code &err)
  {
    logger->error("Session::Err: {}", err.message());
  }
  void Session::on_terminal_initialized()
  {
    logger->info("terminal session initialized");
    send_info("send your first command");
  }
  void Session::on_terminal_data_received(const std::vector<uint8_t> &content)
  {
    auto order = output_response{content};
    auto data = msgpack::pack(order);
    this->write_to_buffer(MessageType::TERMINAL_OUTPUT, data);
  }
  void Session::on_terminal_error(const std::error_code &err)
  {
    on_error(err);
  }
  void Session::start_session(const std::string &container_identifier)
  {
    if (int res = terminal.initialize(container_identifier, "root"); res != 0)
    {
      logger->error("failed to start terminal: err {}", strerror(res));
    }
    else
    {
      terminal.start();
    }
  }

  void Session::send_warning(const std::string &message)
  {
    auto order = session_message{"warning", message};
    auto data = msgpack::pack(order);
    this->write_to_buffer(MessageType::SESSION_MESSAGE, data);
  }

  void Session::send_error(const std::string &message)
  {
    auto order = session_message{"error", message};
    auto data = msgpack::pack(order);
    this->write_to_buffer(MessageType::SESSION_MESSAGE, data);
  }

  void Session::send_info(const std::string &message)
  {
    auto order = session_message{"information", message};
    auto data = msgpack::pack(order);
    this->write_to_buffer(MessageType::SESSION_MESSAGE, data);
  }
  void Session::write_to_buffer(MessageType type, const std::vector<uint8_t> &buffer)
  {
    std::vector<uint8_t> payload(sizeof(uint8_t) + buffer.size());
    auto type_byte = static_cast<uint8_t>(type);
    std::memcpy(payload.data(), &type_byte, sizeof(uint8_t));
    std::memcpy(payload.data() + sizeof(uint8_t), buffer.data(), buffer.size());
    this->connection->write(payload);
  }
} // namespace shell