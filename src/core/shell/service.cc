#include <core/shell/service.h>
#include <definitions.h>
#include <spdlog/spdlog.h>

namespace shell {
  shell::Service::Service(boost::asio::io_service &io_service, int port)
      : io_service(io_service),
        port(port),
        context() {
  }
  bool shell::Service::initialize() {

    websocketpp::lib::error_code ec;
    context.set_reuse_addr(true);
    context.set_access_channels(websocketpp::log::alevel::all);
    context.set_error_channels(websocketpp::log::elevel::all);
    context.set_open_handler(std::bind(&Service::on_open, this, std::placeholders::_1));
    context.set_close_handler(std::bind(&Service::on_close, this, std::placeholders::_1));
    context.set_message_handler(std::bind(&Service::on_payload, this, std::placeholders::_1, std::placeholders::_2));
    context.set_ping_handler(std::bind(&Service::on_ping, this, std::placeholders::_1, std::placeholders::_2));
    context.set_pong_handler(std::bind(&Service::on_pong, this, std::placeholders::_1, std::placeholders::_2));
    context.set_validate_handler(std::bind(&Service::on_validation, this, std::placeholders::_1));
    context.set_fail_handler(std::bind(&Service::on_validation, this, std::placeholders::_1));
    context.init_asio(&io_service, ec);
    if (ec) {
      spdlog::get(LOGGER)->error("{}:{}", __FILE_NAME__, ec.message());
      return false;
    }
    context.listen(port);
    context.start_accept();
    return true;
  }
  void shell::Service::on_open(websocketpp::connection_hdl connection_handler) {
    sessions.try_emplace(connection_handler.lock(), Session::create(context, connection_handler));
  }
  void shell::Service::on_close(websocketpp::connection_hdl connection_handler) {
    sessions.erase(connection_handler.lock());
  }
  void shell::Service::on_payload(websocketpp::connection_hdl connection_handler, service_context::message_ptr msg) {
    sessions.at(connection_handler.lock())->on_payload(msg);
  }
  bool shell::Service::on_ping(websocketpp::connection_hdl connection_handler, std::string str) {
    return sessions.at(connection_handler.lock())->on_ping(str);
  }
  bool shell::Service::on_pong(websocketpp::connection_hdl connection_handler, std::string str) {
    return sessions.at(connection_handler.lock())->on_pong(str);
  }
  bool shell::Service::on_validation(websocketpp::connection_hdl connection_handler) {
    return sessions.at(connection_handler.lock())->on_validation();
  }
  void shell::Service::on_failure(websocketpp::connection_hdl connection_handler) {
    sessions.at(connection_handler.lock())->on_error();
  }
  shell::Service::~Service() {}
} // namespace shell