#include <arpa/inet.h>
#include <core/shell/session.h>
#include <definitions.h>
#include <jail.h>
#include <protocols.pb.h>
#include <spdlog/spdlog.h>
#include <sys/jail.h>
#include <unistd.h>

namespace shell {
  Session::Session(service_context &context, websocketpp::connection_hdl &connection_handler)
      : context(context),
        connection_handler(connection_handler),
        terminal() {
    terminal.on_update([&](const std::string &data) {
      websocketpp::lib::error_code ec;
      std::vector<uint8_t> buffer;
      terminal::TerminalOutput terminalOutPut;
      terminalOutPut.set_data(data);
      write_to_buffer(MessageType::TERMINAL_OUTPUT, terminalOutPut, buffer);
      context.send(connection_handler, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary, ec);
      if (ec) {
        spdlog::get(LOGGER)->error("{}:{}", __FILE_NAME__, ec.message());
      }
    });
  }

  Session::pointer Session::create(service_context &context, websocketpp::connection_hdl &connection_handler) {
    return pointer(new Session(context, connection_handler));
  }

  void Session::on_payload(service_context::message_ptr msg) {
    auto payload = msg->get_payload();
    auto type = static_cast<MessageType> ntohs(*reinterpret_cast<uint16_t *>(payload.data()));
    if (type == MessageType::TERMINAL_INPUT) {
      terminal::TerminalInput terminalInput;
      terminalInput.ParseFromString(payload);
      input_buffer.push_back(terminalInput.data());
      if (terminal.is_available_for_write()) {
        write_to_terminal();
      }
    } else if (type == MessageType::RESIZE_REQUEST) {
      terminal::TerminalResizeRequest terminalResize;
      terminalResize.ParseFromString(payload);
      terminal.resize(terminalResize.columns(), terminalResize.rows());
    } else if (type == MessageType::SESSION_REQUEST) {
      terminal::TerminalShellRequest shellRequest;
      shellRequest.ParseFromString(payload);
      start_session(shellRequest.container());
    }
  }

  bool Session::on_ping(const std::string &data) {
    spdlog::get(LOGGER)->debug("{}:{}", __FILE_NAME__, data);
    return true;
  }

  bool Session::on_pong(const std::string &data) {
    spdlog::get(LOGGER)->debug("{}:{}", __FILE_NAME__, data);
    return true;
  }

  bool Session::on_validation() {
    spdlog::get(LOGGER)->debug("{}:{}", __FILE_NAME__, "validation");
    return true;
  }

  void Session::on_error() {
    spdlog::get(LOGGER)->error("{}:{}", __FILE_NAME__, "error occurred");
  }

  void Session::start_session(const std::string &container_identifier) {
    int jail_id = jail_getid(container_identifier.c_str());
    if (jail_id < 0) {
      //error message to logger
    } else if (jail_attach(jail_id) == -1 || chdir("/") == -1) {
      // error message here as well
    } else {
      auto shell = std::string(getenv("SHELL"));
      if (shell.empty()) {
        shell.append("/bin/sh");
      }
      if (!terminal.initialize(std::string(shell))) {
        return;
      }
    }
  }

  void Session::send_warning(const std::string &message) {
    websocketpp::lib::error_code ec;
    std::vector<uint8_t> buffer;
    terminal::SessionMessage sessionMessage;
    sessionMessage.set_data(message);
    sessionMessage.set_type(::terminal::SessionMessage::Type::SessionMessage_Type_WARNING);
    write_to_buffer(MessageType::SESSION_MESSAGE, sessionMessage, buffer);
    context.send(connection_handler, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary, ec);
    if (ec) {
      spdlog::get(LOGGER)->error("{}:{}", __FILE_NAME__, ec.message());
    }
  }

  void Session::send_error(const std::string &message) {
    websocketpp::lib::error_code ec;
    std::vector<uint8_t> buffer;
    terminal::SessionMessage sessionMessage;
    sessionMessage.set_data(message);
    sessionMessage.set_type(::terminal::SessionMessage::Type::SessionMessage_Type_ERROR);
    write_to_buffer(MessageType::SESSION_MESSAGE, sessionMessage, buffer);
    context.send(connection_handler, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary, ec);
    if (ec) {
      spdlog::get(LOGGER)->error("{}:{}", __FILE_NAME__, ec.message());
    }
  }

  void Session::send_info(const std::string &message) {
    websocketpp::lib::error_code ec;
    std::vector<uint8_t> buffer;
    terminal::SessionMessage sessionMessage;
    sessionMessage.set_data(message);
    sessionMessage.set_type(::terminal::SessionMessage::Type::SessionMessage_Type_INFO);
    write_to_buffer(MessageType::SESSION_MESSAGE, sessionMessage, buffer);
    context.send(connection_handler, buffer.data(), buffer.size(), websocketpp::frame::opcode::binary, ec);
    if (ec) {
      spdlog::get(LOGGER)->error("{}:{}", __FILE_NAME__, ec.message());
    }
  }

  void Session::write_to_buffer(MessageType messageType, MessageLite &message, std::vector<uint8_t> &buffer) {
    const uint16_t type_network = htons(static_cast<uint16_t>(messageType));
    const size_t size = message.ByteSizeLong();
    const uint32_t size_network = htonl(size);
    const size_t length = sizeof(type_network) + sizeof(size_network) + size;
    buffer.resize(length);
    std::memcpy(buffer.data(), &type_network, sizeof(type_network));
    memcpy(buffer.data() + sizeof(type_network), &size_network, sizeof(size_network));
    message.SerializeToArray(buffer.data() + sizeof(type_network) + sizeof(size_network), int(size));
  }

  bool Session::write_to_terminal() {
    if (!terminal.is_available_for_write()) {
      return false;
    }
    std::for_each(input_buffer.begin(), input_buffer.end(), [this](const std::string &data) {
      terminal.write_to_shell(data);
    });
    input_buffer.clear();
    return true;
  }

  Session::~Session() {}
} // namespace shell