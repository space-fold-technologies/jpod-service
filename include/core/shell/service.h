#ifndef __JPOD__WEB_SOCKET_SERVICE__
#define __JPOD__WEB_SOCKET_SERVICE__

#include <core/shell/configuration.h>
#include <core/shell/logger.h>
#include <core/shell/session.h>
#include <map>

namespace shell {

  class Service {
  public:
    Service(boost::asio::io_service &io_service, int port);
    ~Service();
    bool initialize();
    void on_open(websocketpp::connection_hdl connection_handler);
    void on_close(websocketpp::connection_hdl connection_handler);
    void on_payload(websocketpp::connection_hdl connection_handler, service_context::message_ptr msg);
    bool on_ping(websocketpp::connection_hdl connection_handler, std::string str);
    bool on_pong(websocketpp::connection_hdl connection_handler, std::string str);
    bool on_validation(websocketpp::connection_hdl connection_handler);
    void on_failure(websocketpp::connection_hdl connection_handler);

  private:
    boost::asio::io_service &io_service;
    int port;
    service_context context;
    std::map<std::shared_ptr<void>, std::shared_ptr<Session>> sessions;
  };
} // namespace shell
#endif // __JPOD__WEB_SOCKET_SERVICE__