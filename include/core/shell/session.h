#ifndef __JPOD__SHELL_SESSION__
#define __JPOD__SHELL_SESSION__

#include <core/shell/configuration.h>
#include <core/shell/terminal.h>
#include <deque>
#include <google/protobuf/message_lite.h>
#include <memory>
#include <websocketpp/server.hpp>
/*
 
 boost::asio::io_service io_service;
 int fifo_d = open("/tmp/fifo", O_RDONLY);
 boost::asio::posix::stream_descriptor fifo(io_service, fifo_d);
 boost::asio::async_read(fifo, buffer, handler); // <-- you call async_read only once here.
 io_service.run();
 
 */
namespace shell {
  using namespace websocketpp;
  enum MessageType {
    RESIZE_REQUEST = 0,
    TERMINAL_INPUT = 1,
    TERMINAL_OUTPUT = 2,
    SESSION_MESSAGE = 3,
    SESSION_REQUEST = 4
  };
  using google::protobuf::MessageLite;
  class Session : public std::enable_shared_from_this<Session> {
  public:
    typedef std::shared_ptr<Session> pointer;

    ~Session();
    static pointer create(service_context &context, websocketpp::connection_hdl &connection_handler);
    void on_payload(service_context::message_ptr msg);
    bool on_ping(const std::string &data);
    bool on_pong(const std::string &data);
    bool on_validation();
    void on_error();

  private:
    Session(service_context &context, websocketpp::connection_hdl &connection_handler);
    void start_session(const std::string &container_identifier);
    void send_warning(const std::string &message);
    void send_error(const std::string &message);
    void send_info(const std::string &message);
    void write_to_buffer(MessageType messageType, MessageLite &message, std::vector<uint8_t> &buffer);
    bool write_to_terminal();
    service_context &context;
    websocketpp::connection_hdl &connection_handler;
    Terminal terminal;
    std::vector<std::string> input_buffer;
  };

};     // namespace shell
#endif // __JPOD__SHELL_SESSION__