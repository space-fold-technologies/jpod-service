#ifndef __JPOD_SHELL_SERVICE_CONFIGURATION__
#define __JPOD_SHELL_SERVICE_CONFIGURATION__

#include <core/shell/logger.h>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/extensions/permessage_deflate/enabled.hpp>
#include <websocketpp/logger/syslog.hpp>
#include <websocketpp/server.hpp>

namespace shell {
  struct ServiceConfiguration : public websocketpp::config::asio {
    typedef Logger<concurrency_type, websocketpp::log::elevel> elog_type;
    typedef Logger<concurrency_type, websocketpp::log::alevel> alog_type;
  };

  typedef websocketpp::server<ServiceConfiguration> service_context;
} // namespace shell
#endif // __JPOD_SHELL_SERVICE_CONFIGURATION__